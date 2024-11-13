#include "SP/sparse/mot.h"
#include <errno.h>
#include <stdarg.h>
#include <zlib.h>
#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wformat"
#endif
#define MOT_CHECKED_CALLOC(ptr, n, size, on_error)	\
	do												\
	{												\
		if(!((ptr) = calloc(n, size)))				\
		{											\
			errno = MOT_STATUS_NO_MEMORY;			\
			on_error;								\
		}											\
	} while(0)

#define MOT_CHECK_INPUT(_name)				        \
	SPhash hash = 0;								\
	do												\
	{												\
	    if(!tree)                                   \
	    {                                           \
	        errno = MOT_STATUS_INVALID_VALUE;       \
	        return;                                 \
	    }                                           \
	    if(!_name)                                  \
	    {                                           \
	        errno = MOT_STATUS_INVALID_NAME;        \
	        return;                                 \
	    }                                           \
		hash = _mot_sdbm((_name));				    \
		if(hash == 0)                               \
		{                                           \
		    errno = MOT_STATUS_INVALID_NAME;        \
		    return;                                 \
		}                                           \
	} while(0)

#define MOT_READ_GENERIC(dst, n, scanner, fail)     \
	do												\
	{												\
		if(*length < (n))							\
		{											\
			fail;									\
		}											\
		*memory = scanner((dst), *memory, (n));		\
		*length -= n;								\
	} while(0)

#define MOT_COPY_TO_PAYLOAD(tagName) SP_READ_GENERIC(&node->payload.tagName, sizeof(node->payload.tagName), _spSwappedMemscan, goto sp_error)
#define MOT_MAX(a, b) ((a) > (b) ? (a) : (b))

#define ne2be _mot_big_endian_to_native_endian
#define be2ne _mot_big_endian_to_native_endian

struct MOT_node
{
	//written to disk:
	SPhash weight; // 8 bytes
	MOT_tag tag;   // 1 byte
	SPsize length; // 8 bytes

	// encoded in the 7th bit of the tag..
	// signals the node's color..
	SPbool red;

	union
	{
		SPbyte* data;

		// does not hold the root-node
		// but only major and minor branches..
		MOT_tree head;
	} payload; //length bytes

	// total bytes written = 17 + length bytes..
	struct MOT_node* parent;
	struct MOT_node* major;
	struct MOT_node* minor;
};
typedef struct MOT_node MOT_node;

static void _mot_fix_rbt_violations(MOT_node* node, MOT_tree* head);
static MOT_node* _mot_rotate_left(MOT_node* n, MOT_tree* head);
static MOT_node* _mot_rotate_right(MOT_node* n, MOT_tree* head);
static void _mot_delete_bst_impl(MOT_node* n, MOT_tree* head, MOT_node** x, MOT_node** w, MOT_node** r);
static SPbool _mot_is_major(const MOT_node* node);
static SPbool _mot_is_root(const MOT_node* node);
static SPbool _mot_fix_up_rbt(SPbool rBefore, MOT_node* r, MOT_node* x, MOT_node* w, MOT_tree* head);
static SPbool _mot_transplant_rbt(MOT_node* x, MOT_node* w, MOT_tree* head);
static SPbool _mot_transplant_proc_0(MOT_node* x);
static SPbool _mot_transplant_proc_1(MOT_node* x, MOT_node* w, MOT_tree* head);
static SPbool _mot_transplant_proc_2(MOT_node* x, MOT_node* w, MOT_tree* head);
static SPbool _mot_transplant_proc_3(MOT_node* x, MOT_node* w, MOT_tree* head);
static SPbool _mot_transplant_proc_4(MOT_node* x, MOT_node* w, MOT_tree* head);

static int _mot_is_little_endian()
{
	SPuint16 t = 0x0001;
	SPchar c[2];
	memcpy(c, &t, sizeof t);
	return c[0];
}

static void* _mot_swap_bytes(void* s, SPsize length)
{
	for(SPchar *b = s, *e = b + length - 1; b < e; b++, e--)
	{
		SPchar t = *b;
		*b = *e;
		*e = t;
	}
	return s;
}

static void* _mot_big_endian_to_native_endian(void* s, size_t len)
{
	return _mot_is_little_endian() ? _mot_swap_bytes(s, len) : s;
}

static const void* _mot_memcpy(void* dst, const void* src, SPsize n)
{
	memcpy(dst, src, n);
	return (const SPchar*) src + n;
}

static const void* _mot_swapped_memcpy(void* dst, const void* src, SPsize n)
{
	const void* ret = _mot_memcpy(dst, src, n);
	return ne2be(dst, n), ret;
}

static SPbuffer _mot_compress(const void* memory, SPsize length)
{
	const SPsize CHUNK_SIZE = 4096;
	SPbuffer buffer = SP_BUFFER_INIT;

	z_stream stream =
	{
		.zalloc   = Z_NULL,
		.zfree	= Z_NULL,
		.opaque   = Z_NULL,
		.next_in  = (void*) memory,
		.avail_in = length
	};

	if(deflateInit2(
		&stream,
		Z_DEFAULT_COMPRESSION,
		Z_DEFLATED,
		15,
		8,
		Z_DEFAULT_STRATEGY) != Z_OK)
	{
	    errno = MOT_STATUS_COMPRESSION_ERROR;
		return SP_BUFFER_INIT;
	}

    if(stream.avail_in != length)
    {
        errno = MOT_STATUS_COMPRESSION_ERROR;
        return buffer;
    }

	do
	{
		if(spBufferReserve(&buffer, buffer.length + CHUNK_SIZE) != MOT_STATUS_OK)
		{
			errno = MOT_STATUS_NO_MEMORY;
			spBufferFree(&buffer);
			return SP_BUFFER_INIT;
		}

		stream.next_out = buffer.data + buffer.length;
		stream.avail_out = CHUNK_SIZE;

		if(deflate(&stream, Z_FINISH) == Z_STREAM_ERROR)
		{
			errno = MOT_STATUS_COMPRESSION_ERROR;
			spBufferFree(&buffer);
			return SP_BUFFER_INIT;
		}

		buffer.length += CHUNK_SIZE - stream.avail_out;
	} while(!stream.avail_out);

	deflateEnd(&stream);
	return buffer;
}

static SPbuffer _mot_decompress(const void* memory, SPsize length)
{
	const SPsize CHUNK_SIZE = 4096;
	SPbuffer buffer = SP_BUFFER_INIT;

	z_stream stream =
	{
		.zalloc   = Z_NULL,
		.zfree	= Z_NULL,
		.opaque   = Z_NULL,
		.next_in  = (void*) memory,
		.avail_in = length
	};

	if(inflateInit2(&stream, 47) != Z_OK)
	{
	    errno = MOT_STATUS_DECOMPRESSION_ERROR;
		return SP_BUFFER_INIT;
	}

	SPint zlib_ret;
	do
	{
		if(spBufferReserve(&buffer, buffer.length + CHUNK_SIZE))
		{
		    errno = MOT_STATUS_NO_MEMORY;
			spBufferFree(&buffer);
			return SP_BUFFER_INIT;
		}

		stream.avail_out = CHUNK_SIZE;
		stream.next_out = (SPubyte*) buffer.data + buffer.length;

		switch((zlib_ret = inflate(&stream, Z_NO_FLUSH)))
		{
			case Z_MEM_ERROR:
			case Z_DATA_ERROR:
			case Z_NEED_DICT:
			{
				errno = MOT_STATUS_DECOMPRESSION_ERROR;
				spBufferFree(&buffer);
				return SP_BUFFER_INIT;
			}
			default:
				buffer.length += CHUNK_SIZE - stream.avail_out;
		}
	} while(!stream.avail_out);

	if(zlib_ret != Z_STREAM_END)
	{
		errno = MOT_STATUS_DECOMPRESSION_ERROR;
		return SP_BUFFER_INIT;
	}

	inflateEnd(&stream);
	return buffer;
}

static MOT_node* _mot_alloc_node(MOT_tag tag, SPhash name, SPsize length, const SPbyte* value)
{
	MOT_node* node = NULL;
	MOT_CHECKED_CALLOC(node, 1, sizeof(MOT_node), return NULL);
	node->tag = tag;
	node->weight = name;
	node->length = length;
	node->major = node->minor = NULL;
	node->parent = NULL;
	node->red = SP_FALSE;

	if(value && length > 0)
	{
		MOT_CHECKED_CALLOC(node->payload.data, node->length, sizeof(SPbyte), return NULL);
		memcpy(node->payload.data, value, node->length * sizeof(SPbyte));

		if(tag == MOT_TAG_STRING)
			node->payload.data[node->length - 1] = 0;
	}

	return node;
}

// very dangerous, can cause disruption!!
static void _mot_delete_node(MOT_node* n)
{
	if(n)
	{
		if(n->tag != MOT_TAG_ROOT)
		{
			free(n->payload.data);
		}
		else
		{
			motFreeTree(&n->payload.head);
		}
		
		free(n);
	}
}

static SPsize _mot_length_of(MOT_tag tag)
{
	switch(tag)
	{
		case MOT_TAG_BYTE: return sizeof(SPbyte);
		case MOT_TAG_SHORT: return sizeof(SPshort);
		case MOT_TAG_INT: return sizeof(SPint);
		case MOT_TAG_LONG: return sizeof(SPlong);
		case MOT_TAG_FLOAT: return sizeof(SPfloat);
		case MOT_TAG_DOUBLE: return sizeof(SPdouble);
		case MOT_TAG_NULL: break;
	}
	return 0;
}

static SPhash _mot_sdbm_impl(const SPchar *str) 
{
	SPhash hash = 0;
	int c;
	while ((c = *str++))
		hash = c + (hash << 6) + (hash << 16) - hash; // hash * 65599 + c

	return hash;
}

static SPhash _mot_sdbm(const SPchar* str)
{
	SPhash hash = _mot_sdbm_impl(str);
	SPubyte buffer[4];

	for(SPsize i = 0; i < 8; i++)
	{
		buffer[i] = (hash >> (i * 8)) & 0xFF;
	}
	SPhash output = 0;
	memcpy(&output, buffer, sizeof(SPbyte) * 4);
	return output;
}

void _mot_print_binary(const unsigned char *byteArray, size_t size, int level)
{
	for (size_t i = 0; i < size; i++)
	{
		SPubyte byte = byteArray[i];
		for(int i = 0; i < level; i++)
			printf("\t");
		printf("    ");
		for (int j = 7; j >= 0; j--)
			printf("%d", (byte >> j) & 1);
		printf("\n");
	}
}

MOT_tree motAllocTree()
{
	MOT_node* root = NULL;
	MOT_CHECKED_CALLOC(root, 1, sizeof(MOT_node), return NULL);

	root->length = 0LL;
	root->weight = 0LL;
	root->tag = MOT_TAG_NULL;
	root->major = root->minor = NULL;
	return root;
}

void _mot_free_tree_impl(MOT_tree tree)
{
	if(tree)
	{
		if(tree->tag != MOT_TAG_ROOT)
			free(tree->payload.data);
		else
		{
			_mot_free_tree_impl(tree->payload.head);
		}
		_mot_free_tree_impl(tree->major);
		_mot_free_tree_impl(tree->minor);
		free(tree);
	}
}
void motFreeTree(MOT_tree* tree)
{
    _mot_free_tree_impl(*tree);
    *tree = NULL;
}

static MOT_tree _mot_search_for(MOT_tree tree, SPhash hash)
{
	if(tree)
	{
		if(tree->weight == hash)
			return tree;

		MOT_tree node = NULL;
		if((node = _mot_search_for(tree->major, hash)))
			return node;

		if((node = _mot_search_for(tree->minor, hash)))
			return node;
	}
	return NULL;
}

MOT_tree motSearch(const MOT_tree tree, const char* name)
{
	SPhash hash = _mot_sdbm(name);
	return _mot_search_for(tree, hash);
}

static MOT_tree _mot_insert_bytes(MOT_tree* head, MOT_tree node, SPhash weight, MOT_tag tag, SPsize length, const SPbyte* value)
{
	if(!node)
	{
		*head = _mot_alloc_node(tag, weight, length, value);
		return *head;
	}

	if(node->weight == 0 || node->tag == MOT_TAG_NULL)
	{
		SP_ASSERT(node->length == 0LL, "Empty tree cannot have length defined (%lld)", node->length);
		SP_ASSERT(!node->payload.data, "Empty tree cannot have data");
		SP_ASSERT(!node->major && !node->minor, "Empty tree cannot have sub-trees");
		SP_ASSERT(!node->parent, "Empty tree cannot have a parent");
		SP_ASSERT(!node->red, "Empty tree must be black-coded");

		MOT_tree root = _mot_alloc_node(tag, weight, length, value);
		(*head)->weight = weight;
		(*head)->length = root->length;
		(*head)->payload.data = root->payload.data;
		(*head)->tag = tag;

		free(root);
		return *head;
	}

	if(weight == node->weight)
	{
		errno = MOT_STATUS_INVALID_NAME;
		return NULL;
	}

	SPbool maj = (weight > node->weight);
	MOT_tree primary  = maj ? node->major : node->minor;

	if(primary)
		_mot_insert_bytes(head, primary, weight, tag, length, value);
	else
	{
		primary = _mot_alloc_node(tag, weight, length, value);
		primary->parent = node;
		primary->red = SP_TRUE;
		maj ? (node->major = primary) : (node->minor = primary);
		_mot_fix_rbt_violations(primary, head);
	}
	return primary;
}

static MOT_tree _mot_insert_root(MOT_tree* head, MOT_tree node, SPhash weight, MOT_tree value)
{
	if(!node)
	{
		*head = _mot_alloc_node(MOT_TAG_ROOT, weight, 0LL, NULL);
		(*head)->payload.head = value;

		return *head;
	}

	if(node->weight == 0 || node->tag == MOT_TAG_NULL)
	{
		SP_ASSERT(node->length == 0LL, "Empty tree cannot have length defined (%lld)", node->length);
		SP_ASSERT(!node->payload.data, "Empty tree cannot have data");
		SP_ASSERT(!node->major && !node->minor, "Empty tree cannot have sub-trees");
		SP_ASSERT(!node->parent, "Empty tree cannot have a parent");
		SP_ASSERT(!node->red, "Empty tree must be black-coded");

		(*head)->weight = weight;
		(*head)->payload.head = value;
		(*head)->tag = MOT_TAG_ROOT;

		return *head;
	}

	if(value->weight == node->weight)
	{
		errno = MOT_STATUS_INVALID_NAME;
		return NULL;
	}

	SPbool maj = (value->weight > node->weight);
	MOT_tree primary  = maj ? node->major : node->minor;

	if(primary)
		_mot_insert_root(head, primary, weight, value);
	else
	{
		primary = _mot_alloc_node(MOT_TAG_ROOT, value->weight, 0, NULL);
		primary->parent = node;
		primary->payload.head = value;
		primary->red = SP_TRUE;
		maj ? (node->major = primary) : (node->minor = primary);
		_mot_fix_rbt_violations(primary, head);
	}

	return primary;
}

void motInsertTree(MOT_tree* tree, const SPchar* name, MOT_tree value)
{
	MOT_CHECK_INPUT(name);
	if(value && value->tag == MOT_TAG_NULL)
	{
	    errno = MOT_STATUS_INVALID_VALUE;
	    return;
	}

	_mot_insert_root(tree, *tree, hash, value);
}

void motInsertByte(MOT_tree* tree, const SPchar* name, SPbyte value)
{
	MOT_CHECK_INPUT(name);
	_mot_insert_bytes(tree, *tree, hash, MOT_TAG_BYTE, sizeof(SPbyte), (const SPbyte*) &value);
}

void motInsertShort(MOT_tree* tree, const SPchar* name, SPshort value)
{
	MOT_CHECK_INPUT(name);
	_mot_insert_bytes(tree, *tree, hash, MOT_TAG_SHORT, sizeof(SPshort), (const SPbyte*) &value);
}

void motInsertInt(MOT_tree* tree, const SPchar* name, SPint value)
{
	MOT_CHECK_INPUT(name);
	_mot_insert_bytes(tree, *tree, hash, MOT_TAG_INT, sizeof(SPint), (const SPbyte*) &value);
}

void motInsertLong(MOT_tree* tree, const SPchar* name, SPlong value)
{
	MOT_CHECK_INPUT(name);
	_mot_insert_bytes(tree, *tree, hash, MOT_TAG_LONG, sizeof(SPlong), (const SPbyte*) &value);
}

void motInsertFloat(MOT_tree* tree, const SPchar* name, SPfloat value)
{
	MOT_CHECK_INPUT(name);
	_mot_insert_bytes(tree, *tree, hash, MOT_TAG_FLOAT, sizeof(SPfloat), (const SPbyte*) &value);
}

void motInsertDouble(MOT_tree* tree, const SPchar* name, SPdouble value)
{
	MOT_CHECK_INPUT(name);
	_mot_insert_bytes(tree, *tree, hash, MOT_TAG_DOUBLE, sizeof(SPdouble), (const SPbyte*) &value);
}

void motInsertString(MOT_tree* tree, const SPchar* name, const SPchar* value)
{
	if(value)
	{
		MOT_CHECK_INPUT(name);
		_mot_insert_bytes(tree, *tree, hash, MOT_TAG_STRING, strlen(value) + 1, (const SPbyte*)value);
	}
}

void motInsertArray(MOT_tree* tree, const SPchar* name, MOT_tag tag, SPsize length, const void* data)
{
	MOT_tag scalar = tag & ~MOT_TAG_ARRAY;
	if(scalar == MOT_TAG_ROOT)
	{
		errno = MOT_STATUS_INVALID_TAG;
		return;
	}
    if(scalar > MOT_TAG_STRING)
    {
        errno = MOT_STATUS_INVALID_TAG;
        return;
    }
	if(scalar == MOT_TAG_NULL)
	{
		errno = MOT_STATUS_INVALID_TAG;
		return;
	}

	if(!(data && length > 0))
	{
		errno = MOT_STATUS_INVALID_VALUE;
		return;
	}

	MOT_CHECK_INPUT(name);
	if(scalar != MOT_TAG_STRING)
	{
		_mot_insert_bytes(tree, *tree, hash, tag | MOT_TAG_ARRAY, length * _mot_length_of(tag), (const SPbyte*)data);
	}
	else
	{
		SPsize byteCount = length * (sizeof(SPlong) + 1) + 1;
		SPsize actualLength = 0;
		const SPchar** strings = (const SPchar**) data;

		for(SPsize i = 0; i < length; i++)
		{
			const SPchar* string = strings[i];
			SP_ASSERT(string, "Cannot copy from NULL-string");
			byteCount += strlen(string);
		}

		SPchar* memory = NULL;
		MOT_CHECKED_CALLOC(memory, byteCount, sizeof(SPbyte), return);

		SPsize counter = 0, i = 0;
		while(i < byteCount - 1)
		{
			SPsize charsToCopy = strlen(strings[counter]) + 1;
			memcpy(memory + i, &charsToCopy, sizeof(SPlong));
			i += sizeof(SPlong);

			memcpy(memory + i, strings[counter], charsToCopy - 1);
			i += (charsToCopy - 1);

			SPbyte zero = 0;
			memcpy(memory + i, &zero, sizeof(SPbyte));
			i += sizeof(SPbyte);
			counter++;
		}
		memory[byteCount - 1] = 0;
		_mot_insert_bytes(tree, *tree, hash, tag | MOT_TAG_ARRAY, byteCount, (const SPbyte*)memory);
		free(memory);
	}
}

void* motAllocChunk(SPsize size)
{
	SPbyte* ptr = NULL;
	MOT_CHECKED_CALLOC(ptr, size, sizeof(SPbyte), ;);
	return ptr;
}
/*<==========================================================>*
 *  debug
 *<==========================================================>*/

const char* _mot_tag_to_str(MOT_tag tag)
{
	if(tag & MOT_TAG_ARRAY)
	{
		MOT_tag scalar = tag & ~MOT_TAG_ARRAY;
		switch(scalar)
		{
			case MOT_TAG_BYTE: return "byte array";
			case MOT_TAG_SHORT: return "short array";
			case MOT_TAG_INT: return "int array";
			case MOT_TAG_LONG: return "long array";
			case MOT_TAG_FLOAT: return "float array";
			case MOT_TAG_DOUBLE: return "double array";
			case MOT_TAG_STRING: return "string array";
		}
	}
	else
	{
		switch(tag)
		{
			case MOT_TAG_BYTE: return "byte";
			case MOT_TAG_SHORT: return "short";
			case MOT_TAG_INT: return "int";
			case MOT_TAG_LONG: return "long";
			case MOT_TAG_FLOAT: return "float";
			case MOT_TAG_DOUBLE: return "double";
			case MOT_TAG_STRING: return "string";
			case MOT_TAG_ARRAY: return "array";
			case MOT_TAG_ROOT: return "root";
		}
	}
	return "null";
}

void _mot_print_tree(const MOT_tree tree, int level)
{
	if(tree)
	{
		for(int i = 0; i < level; i++)
		printf("\t");

		SPchar color = tree->red ? 'R' : 'B';
		SPchar rank = _mot_is_root(tree) ? '~' : (_mot_is_major(tree) ? '+' : '-');

		if(tree->tag & MOT_TAG_ARRAY)
		{
			MOT_tag scalar = tree->tag & ~MOT_TAG_ARRAY;
			SPsize stride = _mot_length_of(tree->tag & ~MOT_TAG_ARRAY);

			printf("(%c%c) %s (length: %lld stride: %lld) (%lld): ", color, rank, _mot_tag_to_str(tree->tag), tree->length, stride, tree->weight);

			if(
				scalar == MOT_TAG_BYTE ||
				scalar == MOT_TAG_SHORT ||
				scalar == MOT_TAG_INT ||
				scalar == MOT_TAG_LONG
			)
			{
				for(SPsize i = 0; i < tree->length; i += stride)
				{
					SPlong l = 0;
					memcpy(&l, tree->payload.data + i, stride * sizeof(SPbyte));
					printf("%ld ", l);
				}
				printf("\n");
			}

			if(
				scalar == MOT_TAG_FLOAT ||
				scalar == MOT_TAG_DOUBLE
				)
			{
				union
				{
					float _f;
					double _d;
				} value;
				for(SPsize i = 0; i < tree->length; i += stride)
				{
					(scalar == MOT_TAG_FLOAT) ? (value._f = 0.f) : (value._d = 0.0);
					memcpy(((scalar == MOT_TAG_FLOAT) ? (void*)&value._f : (void*)&value._d), tree->payload.data + i, stride);
					printf("%f ", ((scalar == MOT_TAG_FLOAT) ? value._f : value._d));
				}
				printf("\n");
			}

			if(scalar == MOT_TAG_STRING)
			{
				SPsize i = 0;
				while(i < tree->length - 1)
				{
					SPlong size = 0;
					memcpy(&size, tree->payload.data + i, sizeof(SPlong));
					if(size == 0)
						break;

					i += sizeof(SPlong);
					SPchar* str = NULL;
					MOT_CHECKED_CALLOC(str, size, sizeof(SPchar), return);
					memcpy(str, tree->payload.data + i, size);
					printf("\"%s\" ", str);
					free(str);
					i += size;
				}
				printf("\n");
			}
		}
		else
		{
			switch(tree->tag)
			{
				case MOT_TAG_NULL:
				{
					break;
				}

				case MOT_TAG_ROOT:
				{
					printf("(%c%c) root (%lld):\n", color, rank, tree->weight);
					_mot_print_tree(tree->payload.head, level + 1);
					break;
				}
				case MOT_TAG_BYTE:
				case MOT_TAG_SHORT:
				case MOT_TAG_INT:
				case MOT_TAG_LONG:
				{
					SPbyte* buffer = tree->payload.data;
					SP_ASSERT(buffer, "Node has invalid data");

					SPlong value = 0;
					memcpy(&value, buffer, tree->length * sizeof(SPbyte));

					printf("(%c%c) %s (%lld): %ld\n", color, rank, _mot_tag_to_str(tree->tag), tree->weight, value);

					break;
				}

				case MOT_TAG_FLOAT:
				case MOT_TAG_DOUBLE:
				{
					SPbyte* buffer = tree->payload.data;
					SP_ASSERT(buffer, "Node has invalid data");
					union
					{
						float _f;
						double _d;
					} value;
					memcpy(((tree->tag == MOT_TAG_FLOAT) ? (void*)&value._f : (void*)&value._d), buffer, tree->length * sizeof(SPbyte));
					printf("(%c%c) %s (%lld): %f\n", color, rank, _mot_tag_to_str(tree->tag), tree->weight, (tree->tag == MOT_TAG_FLOAT) ? value._f : value._d);
					break;
				}

				case MOT_TAG_STRING:
				{
					SPbyte* buffer = tree->payload.data;
					SP_ASSERT(buffer, "Node has invalid data");
					printf("(%c%c) string (%lld): \"%s\"\n", color, rank, tree->weight, buffer);
					break;
				}
			}
		}
		_mot_print_tree(tree->major, level + 1);
		_mot_print_tree(tree->minor, level + 1);
	}
}
void motPrintTree(const MOT_tree tree)
{
	_mot_print_tree(tree, 0);
	printf("\n~ ... Root\n");
	printf("+ ... Major\n");
	printf("- ... Minor\n");
	printf("B ... Black\n");
	printf("R ... Red\n\n");
	printf("\n");
}

static void _mot_write_bytes(SPbuffer* buffer, const SPubyte* src, SPsize amount, int level, SPbool toNativeEndian)
{
	SPbyte* chunk = motAllocChunk(amount);
	(toNativeEndian) ? _mot_swapped_memcpy(chunk, src, amount) : _mot_memcpy(chunk, src, amount);
	spBufferAppend(buffer, chunk, amount);
#ifdef MOT_PRINT_OUTPUT_DEBUG
	_mot_print_binary(src, amount, level);
#endif
	free(chunk);
}

#ifdef MOT_PRINT_OUTPUT_DEBUG
#define _mot_print_indent(ntabs, msg, ...)\
	do\
	{\
		for(int i = 0; i < ntabs; i++)\
			printf("\t");\
		printf((msg), ##__VA_ARGS__);\
		printf("\n");\
	} while(0)
#else
#define _mot_print_indent(ntabs, msg, ...)
#endif

static void _mot_write_binary(const MOT_tree tree, SPbuffer* buffer, int level)
{
	if(!tree)
	{
		//signal that this node is done..
		_mot_print_indent(level, "(end)");

		//encode the tag
		MOT_tag null = MOT_TAG_NULL;
		_mot_write_bytes(buffer, (const SPubyte*) &null, sizeof(SPbyte), level, SP_FALSE);
		return;
	}

	//encode the tag..
	SPuint8 tag = tree->tag | ((tree->red & 1) << 6);
	_mot_print_indent(level, "tag: (%s)", _mot_tag_to_str(tree->tag));
	_mot_write_bytes(buffer, (const SPubyte*) &tag, sizeof(SPbyte), level, SP_FALSE);

	//encode the weight..
	_mot_print_indent(level, "weight: %lld", tree->weight);
	_mot_write_bytes(buffer, (const SPubyte*) &tree->weight, sizeof(SPlong), level, SP_TRUE);

	if(tree->tag == MOT_TAG_STRING || (tree->tag & MOT_TAG_ARRAY))
	{
		//write the length of the string
		_mot_print_indent(level, "length: %lld", tree->length);
		_mot_write_bytes(buffer, (const SPubyte*) &tree->length, sizeof(SPsize), level, SP_TRUE);
	}

	if(tree->tag != MOT_TAG_ROOT)
	{
		SP_ASSERT(tree->payload.data, "Node %lld has invalid data to write", tree->weight);
		if((tree->tag & MOT_TAG_ARRAY) == 0)
		{
			_mot_print_indent(level, "data: (%lld byte(s))", tree->length);
			_mot_write_bytes(buffer, (const SPubyte*)tree->payload.data, tree->length, level, tree->tag != MOT_TAG_STRING);
		}
		else
		{
			if((tree->tag & ~MOT_TAG_ARRAY) == MOT_TAG_STRING)
			{
				_mot_print_indent(level, "data: (%lld byte(s))", tree->length);

				SPsize i = 0;
				while(i < tree->length - 1)
				{
					SPlong length = 0;
					memcpy(&length, tree->payload.data + i, sizeof(SPlong));
					_mot_write_bytes(buffer, (const SPubyte*) &length, sizeof(SPlong), level, SP_TRUE);
					i += sizeof(SPlong);
					_mot_write_bytes(buffer, (const SPubyte*) tree->payload.data + i, length, level, SP_FALSE);
					i += length;
				}
				SPubyte zero = 0;
				_mot_write_bytes(buffer, (const SPubyte*) &zero, sizeof(SPbyte), level, SP_FALSE);
			}
			else
			{
				_mot_print_indent(level, "data: (%lld byte(s))", tree->length);
				SPsize stride = _mot_length_of(tree->tag & ~MOT_TAG_ARRAY);
				for(SPsize i = 0; i < tree->length / stride; i++)
					_mot_write_bytes(buffer, (const SPubyte*)tree->payload.data + stride * i, stride, level, SP_TRUE);
			}
		}
	}
	else
	{
		_mot_print_indent(level + 1, "root");
        _mot_write_binary(tree->payload.head, buffer, level + 1);
	}
	_mot_print_indent(level + 1, "major");
	_mot_write_binary(tree->major, buffer, level + 1);

	_mot_print_indent(level + 1, "minor");
	_mot_write_binary(tree->minor, buffer, level + 1);
}

/*
 *------------------------------------------------------------------------------------------------------------------
 *   writing rule
 *------------------------------------------------------------------------------------------------------------------
 *  for all nodes:
 *  1. write current tag..
 *  2. write weight..
 *  3. write data (explained below)..
 *  4. for major and minor repeat step 1 if not NULL, otherwise write MOT_TAG_NULL as tag respectively..
 *
 *  for root nodes:
 *  skip step 3..
 *
 *  for scalar nodes:
 *  write data..
 *  (you neither need length nor stride, since type is known by tag)..
 *
 *  for number array nodes:
 *  write length..
 *  write data..
 *  (stride is known by tag, counts for byte, short, int long, float double and string)..
 *
 *  for string array nodes:
 *  write length (how many strings)..
 *  write length (string length)..
 *  write data (char data)..
 *  write null (end)..
 *  due to this, the number types have to be known, therefore more concrete tags needed..
*/
SPbuffer motWriteBinary(const MOT_tree tree)
{
	SPbuffer buffer = SP_BUFFER_INIT;
	if(!tree)
	{
		errno = MOT_STATUS_INVALID_VALUE;
		return buffer;
	}
	_mot_write_binary(tree, &buffer, 0);
	SPbuffer compressed = _mot_compress(buffer.data, buffer.length);
	spBufferFree(&buffer);
	return compressed;
}

static MOT_tree _mot_read_binary(const SPubyte** memory, SPsize* length)
{
	SPubyte tag;
	MOT_READ_GENERIC(&tag, sizeof(SPubyte), _mot_memcpy, return NULL);

	SPbool redness = (tag >> 6) & 1;
	tag &= ~0x40;

	if(tag == MOT_TAG_NULL)
		return NULL;

	MOT_tree tree;
	MOT_CHECKED_CALLOC(tree, 1, sizeof(MOT_node), return NULL);
	MOT_READ_GENERIC(&tree->weight, sizeof(SPlong), _mot_swapped_memcpy, return NULL);
	tree->red = redness;
	tree->tag = tag;

	if(tag != MOT_TAG_ROOT)
	{
		if(!(tag & MOT_TAG_ARRAY) && tag != MOT_TAG_STRING)
		{
			tree->length = _mot_length_of(tag);
			if(tree->length)
			{
				MOT_CHECKED_CALLOC(tree->payload.data, tree->length, sizeof(SPbyte), return NULL);
				MOT_READ_GENERIC(tree->payload.data, tree->length, _mot_swapped_memcpy, return NULL);
			}
		}
		else if(tag == MOT_TAG_STRING || tag == (MOT_TAG_BYTE | MOT_TAG_ARRAY))
		{
			MOT_READ_GENERIC(&tree->length, sizeof(SPlong), _mot_swapped_memcpy, return NULL);
			if(tree->length)
			{
				MOT_CHECKED_CALLOC(tree->payload.data, tree->length, sizeof(SPbyte), return NULL);
				MOT_READ_GENERIC(tree->payload.data, tree->length, _mot_memcpy, return NULL);
			}
		}
		else
		{
			MOT_READ_GENERIC(&tree->length, sizeof(SPlong), _mot_swapped_memcpy, return NULL);
			if(tree->length)
			{
				MOT_CHECKED_CALLOC(tree->payload.data, tree->length, sizeof(SPbyte), return NULL);
				if((tag & ~MOT_TAG_ARRAY) != MOT_TAG_STRING)
				{
					SPsize stride = _mot_length_of(tag & ~MOT_TAG_ARRAY);
					for(SPsize i = 0; i < tree->length / stride; i++)
						MOT_READ_GENERIC(tree->payload.data + i * stride, stride, _mot_swapped_memcpy, return NULL);
				}
				else
				{
					SPsize i = 0;
					while(i < tree->length - 1)
					{
						SPlong size = 0;
						MOT_READ_GENERIC(&size, sizeof(SPlong), _mot_swapped_memcpy, return NULL);
						memcpy(tree->payload.data + i, (const SPchar*) &size, sizeof(SPlong));
						i += sizeof(SPlong);

						MOT_READ_GENERIC(tree->payload.data + i, size, _mot_memcpy, return NULL);
						i += size;
					}

					SPbyte zero;
					MOT_READ_GENERIC(&zero, sizeof(SPbyte), _mot_memcpy, return NULL);
					tree->payload.data[tree->length - 1] = zero;
				}
			}
		}
	}
	else
	{
		tree->payload.head = _mot_read_binary(memory, length);
	}
	tree->major = _mot_read_binary(memory, length);
	tree->minor = _mot_read_binary(memory, length);

	if(tree->major)
	    tree->major->parent = tree;
	if(tree->minor)
		tree->minor->parent = tree;

	return tree;
}

/*
 *------------------------------------------------------------------------------------------------------------------
 *   reading rule
 *------------------------------------------------------------------------------------------------------------------
 *  for all nodes:
 *  1. read tag..
 *  2. read weight..
 *  3. read data..
 *  4. for major and minor, repeat step 1 (if not 0 as tag, respectively)..
 *
 *  for branch nodes:
 *  skip step 3..
 *
 *  for scalar nodes:
 *  read n bytes (length known by tag, counts for byte, short, int, long, float, double)..
 *
 *  for number array nodes:
 *  read m × n bytes, where m is the length of the array and n is the stride..
 *  (stride known by tag, counts for byte, short, int, long, float, double and string)..
 *
 *  for string array nodes:
 *  read array length..
 *  read string length..
 *  read n bytes..
 *  repeat step 1 until NULL is found..
 *
 *  reading stops when a tag of 0 is found..
*/

MOT_tree motReadBinary(SPbuffer buffer)
{
	SPbuffer decompressed = _mot_decompress(buffer.data, buffer.length);
	const SPubyte** memory = (const SPubyte**) &decompressed.data;
	SPsize length = decompressed.length;
	
	return _mot_read_binary(memory, &length);
}

/*<==========================================================>*
 *  error signaling
 *<==========================================================>*/
MOT_status motGetLastError()
{
    return errno;
}

const char* motGetErrorInfo(MOT_status status)
{
    switch(status)
    {
        case MOT_STATUS_NO_MEMORY: return "Your local machine has failed to allocate memory";
        case MOT_STATUS_COMPRESSION_ERROR: return "Error while compressing data";
        case MOT_STATUS_DECOMPRESSION_ERROR: return "Error while decompressing data";
        case MOT_STATUS_INVALID_NAME: return "Given name was either empty, NULL, or already assigned to a node";
        case MOT_STATUS_INVALID_VALUE: return "Given value was invalid";
        case MOT_STATUS_INVALID_TAG: return "Given tag was invalid";
    }
    return "No errors";
}

/*<==========================================================>*
 *  tree operations
 *<==========================================================>*/

static MOT_node* _mot_find_max(MOT_node* n)
{
	return n ? (!n->major ? n : _mot_find_max(n->major)) : NULL;
}

static MOT_node* _mot_find_min(MOT_node* n)
{
	return n ? (!n->minor ? n : _mot_find_min(n->minor)) : NULL;
}

static SPbool _mot_is_major(const MOT_node* node)
{
	return (node && node->parent) ? (node->parent->major == node) : SP_FALSE;
}

static SPbool _mot_is_root(const MOT_node* node)
{
	return (node && !node->parent);
}

static MOT_node* _mot_find_member(MOT_node* node)
{
	SP_ASSERT(!_mot_is_root(node->parent), "Oopsie, this should not have happened");
	return _mot_is_major(node->parent) ? node->parent->parent->minor : node->parent->parent->major;
}

static SPsize _mot_calculate_black_depth(const MOT_node* rbt)
{
	if(!rbt)
		return 1;
	
	SPsize count = 0;
	if(!rbt->red)
		count++;
	SPsize majorDepth = _mot_calculate_black_depth(rbt->major);
	SPsize minorDepth = _mot_calculate_black_depth(rbt->minor);
	return count + MOT_MAX(minorDepth, majorDepth);
}

static SPbool _mot_verify_rbt_impl(const MOT_node* rbt, SPsize depth, SPsize ref)
{
	if(rbt)
	{
		if(rbt->red)
		{
			if(!rbt->parent)
				return SP_FALSE;
			else
			{
				if(rbt->parent->red)
					return SP_FALSE;
			}
		}
		else
			++depth;
		
		if(rbt->major || rbt->minor)
		{
			return _mot_verify_rbt_impl(rbt->major, depth, ref) && _mot_verify_rbt_impl(rbt->minor, depth, ref);
		}
		else
		{
			return !rbt->red ? (ref == depth) : SP_TRUE;
		}
	}
	return SP_TRUE;
}

SPbool motVerifyRBT(MOT_node* rbt)
{
	SPsize depth = _mot_calculate_black_depth(rbt) - 1;
	return _mot_verify_rbt_impl(rbt, 0, depth);
}

static void _mot_fix_rbt_violations(MOT_node* node, MOT_tree* head)
{
	if(node)
	{
		if(node->red && node->parent->red)
		{
			MOT_node* m = _mot_find_member(node);
			MOT_node* p = node->parent;
			MOT_node* g = p->parent;
			MOT_node* r = NULL;
			
			SPbool isRed = m != NULL ? m->red : SP_FALSE;
			
			if(isRed)
			{
				// color switch..
				p->red = !p->red;
				g->red = g->parent ? !g->red : SP_FALSE;
				m->red = !m->red;
				r = g;
			}
			else
			{
				/*
				 *  child-parent combinations:
				 *
				 *  major = true..
				 *  minor = false
				 *
				 *  P   C | rotation
				 * -------|----------
				 *  0   0 |	R
				 *  0   1 |	LR
				 *  1   0 |	RL
				 *  1   1 |	L
				 *
				 *	for skews, rotate once..
				 *	for triangles, rotate twice..
				 */
				 SPbool pMajor = _mot_is_major(p);
				 SPbool nMajor = _mot_is_major(node);
				 
				 if(!pMajor && !nMajor)
				 {
					// R-rotation..
					 r = _mot_rotate_right(node->parent->parent, head);
				 }
				 else if(!pMajor && nMajor)
				 {
					// LR-rotation..
					r = _mot_rotate_left(node->parent, head);
						_mot_rotate_right(node->parent, head);
				 }
				 else if(pMajor && !nMajor)
				 {
					// RL-rotation..
					r = _mot_rotate_right(node->parent, head);
						_mot_rotate_left(node->parent, head);
				 }
				 else
				 {
					// L-rotation..
					r = _mot_rotate_left(node->parent->parent, head);
				 }
				 r->red = !r->red;
				 pMajor ? (r->minor->red = !r->minor->red) : (r->major->red = !r->major->red);
			}
			_mot_fix_rbt_violations(r, head);
		}
	}
}

static MOT_node* _mot_rotate_left(MOT_node* n, MOT_tree* head)
{
	SP_ASSERT(n, "Expected rotation node");
	SPbool maj = _mot_is_major(n);
	MOT_node* p = n->parent;
	MOT_node* m = n->major;
	MOT_node* c = m ? n->major->minor : NULL;
	
	n->parent = m;
	n->major->minor = n;
	
	if(c)
	   c->parent = n;
	n->major = c;
	
	if(m)
	   m->parent = p;
	
	p ? (maj ? (p->major = m) : (p->minor = m)) : (*head = m);
	return m;
}

static MOT_node* _mot_rotate_right(MOT_node* n, MOT_tree* head)
{
	SP_ASSERT(n, "Expected rotation node");
	SPbool maj = _mot_is_major(n);
	MOT_node* p = n->parent;
	MOT_node* m = n->minor;
	MOT_node* c = m ? n->minor->major : NULL;
	
	n->parent = m;
	n->minor->major = n;
	
	if(c)
	   c->parent = n;
	n->minor = c;
	
	if(m)
	   m->parent = p;
	
	p ? (maj ? (p->major = m) : (p->minor = m)) : (*head = m);
	return m;
}

static void _mot_delete_bst_impl(MOT_node* n, MOT_tree* head, MOT_node** _r, MOT_node** _x, MOT_node** _w)
{
	if(n)
	{
		MOT_node* x = NULL;
		MOT_node* w = NULL;
		MOT_node* r = NULL;
		MOT_node* p = n->parent;
		
		SPbool maj = _mot_is_major(n);
		SPbool root = _mot_is_root(n);
		
		if(!n->major && !n->minor)
		{
			if(p)
			{
				maj ? (p->major = NULL) : (p->minor = NULL);
				w = maj ? (p->minor) : (p->major);
			}
			else
			{
				SP_ASSERT(root, "Fatal, expected deleted node to be root");
			}
		}
		else if((n->major && !n->minor) || (!n->major && n->minor))
		{
			r = n->major ? n->major : n->minor;
			SP_ASSERT(r, "Fatal, expected to have replacement");
			r->parent = p;
			
			if(maj)
			{
				if(p)
				   p->major = r;
				x = r->major;
				w = r->minor;
			}
			else
			{
				if(p)
				   p->minor = r;
				x = r->minor;
				w = r->major;
			}
		}
		else
		{
#ifndef MOT_HAVE_BST_MAJOR_INCLINED
			r = _mot_find_min(n->major);
			w = r->parent ? (_mot_is_major(r) ? r->parent->minor : r->parent->major) : NULL;
			
			SP_ASSERT(r, "Expected to have replacement");
			SP_ASSERT(!r->minor, "Expected to have minimum replacement");
			MOT_node *q = n->minor;
			MOT_node *m = n->major;
			MOT_node *a = r->parent;
			MOT_node *b = r->major;
			
			SP_ASSERT(!r->minor, "Expected to have no more minimum");
			
			if(p)
			   maj ? (p->major = r) : (p->minor = r);
			r->parent = p;
		   
			if(q)
			   q->parent = r;
			r->minor = q;
		  
			if(r != m)
			{
				SP_ASSERT(a, "Expected minimum to have parent");
				
				// link m and r..
				r->major = m;
				m->parent = r;
				
				// link a and b..
				a->minor = b;
				if(b)
				   b->parent = a;
			}
#else
			r = _mot_find_max(n->minor);
			w = r->parent ? (_mot_is_major(r) ? r->parent->minor : r->parent->major) : NULL;
			SP_ASSERT(r, "Expected to have replacement");
			SP_ASSERT(!r->major, "Expected to maximum replacement");
			
			MOT_node *q = n->major;
			MOT_node *m = n->minor;
			MOT_node *a = r->parent;
			MOT_node *b = r->minor;
			
			SP_ASSERT(!r->major, "Expected to have no more maxima");
			if(p)
				maj ? (p->major = r) : (p->minor = r);
			r->parent = p;
			
			if(q)
			   q->parent = r;
			r->major = q;
			if(r != m)
			{
				// link m and r..
				r->minor = m;
				m->parent = r;
				
				// link a and b..
				a->major = b;
				if(b)
				   b->parent = a;
			}
#endif
			x = b;
		}
		
		*_r = r;
		*_x = x;
		*_w = w;
		
		if(!p)
		   *head = r;
	   
		_mot_delete_node(n);
	}
}

SPbool motDelete(MOT_tree* tree, const SPchar* name)
{
	MOT_node* n = motSearch(*tree, name);
	if(n)
	{
		SPbool red = n->red;
		MOT_node* r = NULL;
		MOT_node* x = NULL;
		MOT_node* w = NULL;
		_mot_delete_bst_impl(n, tree, &r, &x, &w);
		
		if(x && w)
		{
			SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
		}
		return _mot_fix_up_rbt(red, r, x, w, tree);
	}
	return SP_FALSE;
}

static SPbool _mot_fix_up_rbt(SPbool redBefore, MOT_node* r, MOT_node* x, MOT_node* w, MOT_tree* head)
{
	SPbool redAfter = r ? r->red : SP_FALSE;
	if(redBefore && redAfter)
	{
		return SP_TRUE;
	}
	
	if(!redBefore && redAfter)
	{
		if(r)
		   r->red = SP_FALSE;
	   return SP_TRUE;
	}
	
	if(redBefore && !redAfter)
	{
		if(r)
		   r->red = SP_TRUE;
	}
	
	if(_mot_is_root(x))
		return SP_TRUE;
	
	return _mot_transplant_rbt(x, w, head);
}

static SPbool _mot_transplant_rbt(MOT_node* x, MOT_node* w, MOT_tree* head)
{
	SPbool xRed = x ? x->red : SP_FALSE;
	if(xRed)
	{
		return _mot_transplant_proc_0(x);
	}
	else
	{
		if(!w)
		{
			// following cases would expect w to have children..
	        // since double black cannot have children, return here..
			return SP_TRUE;
		}
		
		SPbool maj = !_mot_is_major(w);
		if(w->red)
		{
			// x is black and w is red..
			return _mot_transplant_proc_1(x, w, head);
		}
		else
		{
			SPbool wMjB = w->major ? !w->major->red : SP_TRUE;
			SPbool wMnB = w->minor ? !w->minor->red : SP_TRUE;
			
			if(wMjB && wMnB)
			{
				return _mot_transplant_proc_2(x, w, head);
			}
			
			SPbool c = maj ? (!wMjB && wMnB) : (!wMnB && wMjB);
			if(c)
			{
				return _mot_transplant_proc_3(x, w, head);
			}
			
			c = maj ? !wMnB : !wMjB;
			if(c)
			{
				return _mot_transplant_proc_4(x, w, head);
			}
		}
	}
	
	return SP_FALSE;
}

SPbool _mot_transplant_proc_0(MOT_node* x)
{
	if(x->red)
	{
		x->red = SP_FALSE;
		return SP_TRUE;
	}
	
	return SP_FALSE;
}

SPbool _mot_transplant_proc_1(MOT_node* x, MOT_node* w, MOT_tree* head)
{
	if(w)
	{
		SPbool xBlack = x ? !x->red : SP_TRUE;
		SPbool maj = !_mot_is_major(w);
		if(xBlack && w->red)
		{
			// only x could be double black, w must be red..
			SP_ASSERT(w->parent, "Replacement expected to have parent");
            SP_ASSERT(maj ? w->parent->minor == w : w->parent->major == w, "Linking error");
			if(x)
            {
                SP_ASSERT(w->parent == x->parent, "Replacement and sibling expected to have equal parent");
            }
			
			w->red = SP_FALSE;
			w->parent->red = SP_TRUE;
			MOT_node* m = maj ? w->major : w->minor;
			MOT_node* p = maj ? _mot_rotate_right(w->parent, head) : _mot_rotate_left(w->parent, head);
			SP_ASSERT(p, "Expeceted to have rotation replacement");
            SP_ASSERT(p == w, "Rotation error");
			p = maj ? p->major : p->minor;
			SP_ASSERT(p, "Expected to have parent");
			x = maj ? p->major : p->minor;
			w = maj ? p->minor : p->major;
			SP_ASSERT(m == w, "Rotation error");
			return _mot_transplant_rbt(x, w, head);
		}
	}
	return SP_FALSE;
}

SPbool _mot_transplant_proc_2(MOT_node* x, MOT_node* w, MOT_tree* head)
{
	if(w)
	{
		// x is black and w is black..
		// x could be double black, w cannot be double black..
		SPbool xBlack = x ? !x->red : SP_TRUE;
		if(xBlack && !w->red)
		{
			// only x could be double black..
			SPbool maj = !_mot_is_major(w);
			SPbool wMnB = w->minor ? !w->minor->red : SP_TRUE;
			SPbool wMjB = w->major ? !w->major->red : SP_TRUE;
			if(wMnB && wMjB)
			{
				w->red = SP_TRUE;
				x = w->parent;
				if(_mot_is_root(x))
				{
					x->red = SP_FALSE;
					return SP_TRUE;
				}
				maj = _mot_is_major(x);
				w = maj ? x->parent->minor : x->parent->major;
				SP_ASSERT(x, "Replacement expected to have parent");
				if(x->red)
				{
					x->red = SP_FALSE;
					return SP_TRUE;
				}
				else
				{
					w = maj ? w->parent->minor : w->parent->major;
					x = maj ? w->parent->major : w->parent->minor;
					return _mot_transplant_rbt(x, w, head);
				}
			}
		}
	}
	return SP_FALSE;
}

SPbool _mot_transplant_proc_3(MOT_node* x, MOT_node* w, MOT_tree* head)
{
	if(w)
	{
		SPbool xBlack = x ? !x->red : SP_TRUE;
		SPbool wMjB = w->major ? !w->major->red : SP_TRUE;
		SPbool wMnB = w->minor ? !w->minor->red : SP_TRUE;
		SPbool maj = !_mot_is_major(w);
		SPbool c = maj ? (!wMjB && wMnB) : (wMjB && !wMnB);
		
		if(xBlack && c)
		{
			if(x)
			{
				SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
			}
			maj = !_mot_is_major(w);
			SP_ASSERT(maj ? w->major : w->minor, "Expected sibling's child");
			maj ? (w->major->red = SP_FALSE) : (w->minor->red = SP_FALSE);
			w->red = SP_TRUE;
			w = maj ? _mot_rotate_left(w, head) : _mot_rotate_right(w, head);
			SP_ASSERT(w, "Expected sibling");
            SP_ASSERT(w->parent, "Expected parent");
			x = maj ? w->parent->major : w->parent->minor;
			SP_ASSERT(w != x, "Sibling and replacement cannot be the same");
			return _mot_transplant_proc_4(x, w, head);
		}
	}
	return SP_FALSE;
}

SPbool _mot_transplant_proc_4(MOT_node* x, MOT_node* w, MOT_tree* head)
{
	if(w)
	{
		// x can be double black..
		// w cannot be double black..
		SPbool wMjR = w->major ? w->major->red : SP_FALSE;
		SPbool wMnR = w->minor ? w->minor->red : SP_FALSE;
		SPbool xBlack = x ? !x->red : SP_TRUE;
		SPbool maj = !_mot_is_major(w);
		SPbool c = maj ? wMnR : wMjR;
		
		if(xBlack && c)
		{
			if(x)
            {
                SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
            }
			SP_ASSERT(w->parent, "Expected to have parent");
			w->red = w->parent->red;
			w->parent->red = SP_FALSE;
			SP_ASSERT(maj ? w->minor : w->major, "Expected sibling child");
			maj ? (w->minor->red = SP_FALSE) : (w->major->red = SP_FALSE);
			maj ? _mot_rotate_right(w->parent, head) : _mot_rotate_left(w->parent, head);
			return SP_TRUE;
		}
	}
	return SP_FALSE;
}

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic pop
#endif