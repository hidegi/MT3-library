#include "SP/sparse/mot.h"
#include <errno.h>
#include <stdarg.h>
#include <zlib.h>

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

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wformat"
#endif
#define MOT_CHECKED_CALLOC(ptr, n, size, on_error)	\
	do						\
	{						\
		if(!((ptr) = calloc(n, size)))		\
		{					\
			errno = MOT_ERR_MEM;		\
			on_error;			\
		}					\
	} while(0)

#define MOT_CHECKED_APPEND(b, ptr, len)		        \
    do				     			\
    {				              		\
    if(spBufferAppend((b), (ptr), (len)))           	\
	return MOT_ERR_MEM;		          	\
    } while(0)

#define MOT_DUMP_NUM(type, x)					\
	do							\
	{							\
		type temp = x;			         	\
		ne2be(&temp, sizeof temp);		  	\
		MOT_CHECKED_APPEND(b, &temp, sizeof temp);	\
	} while(0)


#define MOT_CHECK_INPUT(_name)						\
	SPhash hash = 0;						\
	do								\
	{								\
		SP_ASSERT(tree, "Cannot Insert data to empty tree");	\
		SP_ASSERT((_name), "Node must have a name");		\
		hash = _mot_sdbm((_name));				\
		SP_ASSERT(hash != 0, "Name cannot be empty");		\
	} while(0)

#define MOT_READ_GENERIC(dst, n, scanner, on_failure)	\
	do						\
	{						\
		if(*length < (n))			\
		{					\
			on_failure;			\
		}					\
		*memory = scanner((dst), *memory, (n));	\
		*length -= n;				\
	} while(0)

#define MOT_COPY_TO_PAYLOAD(tagName) SP_READ_GENERIC(&node->payload.tagName, sizeof(node->payload.tagName), _spSwappedMemscan, goto sp_error)

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
		.zfree    = Z_NULL,
		.opaque   = Z_NULL,
		.next_in  = (void*) memory,
		.avail_in = length
	};

	int windowbits = 15;
	if(deflateInit2(
		&stream,
		Z_DEFAULT_COMPRESSION,
		Z_DEFLATED,
		windowbits,
		8,
		Z_DEFAULT_STRATEGY) != Z_OK)
	{
		SP_WARNING("Failed to initialize zlib");
		return SP_BUFFER_INIT;
	}

	SP_ASSERT(stream.avail_in == length, "Available input does not match length");
	do
	{
		if(spBufferReserve(&buffer, buffer.length + CHUNK_SIZE) != MOT_ERR_NONE)
		{
			SP_DEBUG("Failed to reserve buffer");
			spBufferFree(&buffer);
			return SP_BUFFER_INIT;
		}

		stream.next_out = buffer.data + buffer.length;
		stream.avail_out = CHUNK_SIZE;

		if(deflate(&stream, Z_FINISH) == Z_STREAM_ERROR)
		{
			SP_WARNING("Failed to write data");
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
		.zfree    = Z_NULL,
		.opaque   = Z_NULL,
		.next_in  = (void*) memory,
		.avail_in = length
	};

	if(inflateInit2(&stream, 47) != Z_OK)
	{
		SP_WARNING("Failed to initialize zlib");
		return SP_BUFFER_INIT;
	}

	SPint zlib_ret;
	do
	{
		if(spBufferReserve(&buffer, buffer.length + CHUNK_SIZE))
		{
			SP_DEBUG("Failed to reserve buffer");
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
				SP_WARNING("Error while decompressing");
				spBufferFree(&buffer);
				return SP_BUFFER_INIT;
			}
			default:
				buffer.length += CHUNK_SIZE - stream.avail_out;
		}
	} while(!stream.avail_out);

	if(zlib_ret != Z_STREAM_END)
	{
		SP_WARNING("Error while decompressing");
		return SP_BUFFER_INIT;
	}

	inflateEnd(&stream);
	return buffer;
}
static SPbool _mot_is_major(const struct MOT_node* node)
{
	return (node && node->parent) ? (node->parent->major == node) : SP_FALSE;
}

static SPbool _mot_is_root(const struct MOT_node* node)
{
	return (node && !node->parent);
}

static struct MOT_node* _mot_find_member(struct MOT_node* node)
{
	SP_ASSERT(!_mot_is_root(node->parent), "Oopsie, this should not have happened");
	return _mot_is_major(node->parent) ? node->parent->parent->minor : node->parent->parent->major;
}

static struct MOT_node* _mot_rotate_left(struct MOT_node* n, struct MOT_node** head)
{
    SPbool isMajor = _mot_is_major(n);
	struct MOT_node* p = n->parent;
	struct MOT_node* m = n->major;
	struct MOT_node* c = n->major->minor;
	
	n->parent = m;
	n->major->minor = n;
	if(c)
	   c->parent = n;
   
	n->major = c;
	m->parent = p;
	
	p ? (isMajor ? (p->major = m) : (p->minor = m)) : (*head = m);
	return m;
}

static struct MOT_node* _mot_rotate_right(struct MOT_node* n, struct MOT_node** head)
{
	SPbool isMajor = _mot_is_major(n);
	struct MOT_node* p = n->parent;
	struct MOT_node* m = n->minor;
	struct MOT_node* c = n->minor->major;
	
	n->parent = m;
	n->minor->major = n;
	if(c)
	   c->parent = n;
   
	n->minor = c;
	m->parent = p;
	
	p ? (isMajor ? (p->major = m) : (p->minor = m)) : (*head = m);
	return m;
}

static void _mot_fix_rbt_violations(struct MOT_node* node, struct MOT_node** head)
{
	if(node)
	{
		if(node->red && node->parent->red)
		{
			struct MOT_node* m = _mot_find_member(node);
			struct MOT_node* p = node->parent;
			struct MOT_node* g = p->parent;
			struct MOT_node* r = NULL;
			
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
				 *  0   0 |    R
				 *  0   1 |    LR
				 *  1   0 |    RL
				 *  1   1 |    L
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

static struct MOT_node* _mot_alloc_node(MOT_tag tag, SPhash name, SPsize length, const SPbyte* value)
{
	struct MOT_node* node = NULL;
	MOT_CHECKED_CALLOC(node, 1, sizeof(struct MOT_node), return NULL);
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
	struct MOT_node* root = NULL;
	MOT_CHECKED_CALLOC(root, 1, sizeof(struct MOT_node), return NULL);

    root->length = 0LL;
	root->weight = 0LL;
	root->tag = MOT_TAG_NULL;
	root->major = root->minor = NULL;
	return root;
}

void motFreeTree(MOT_tree tree)
{
	if(tree)
	{
		if(tree->tag != MOT_TAG_ROOT)
			free(tree->payload.data);
		else
		{
			motFreeTree(tree->payload.head);
		}
		motFreeTree(tree->major);
		motFreeTree(tree->minor);
		free(tree);
	}
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

MOT_tree motSearch(MOT_tree tree, const char* name)
{
	long long hash = _mot_sdbm(name);
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
		SP_DEBUG("Name has already been given to a node");
		return NULL;
	}

	SPbool isMajor = (weight > node->weight);
	MOT_tree primary  = isMajor ? node->major : node->minor;

	if(primary)
		_mot_insert_bytes(head, primary, weight, tag, length, value);
	else
	{
		primary = _mot_alloc_node(tag, weight, length, value);
		primary->parent = node;
		primary->red = SP_TRUE;
		isMajor ? (node->major = primary) : (node->minor = primary);
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
		SP_DEBUG("Name has already been given to a node");
		return NULL;
	}

	SPbool isMajor = (value->weight > node->weight);
	MOT_tree primary  = isMajor ? node->major : node->minor;

	if(primary)
		_mot_insert_root(head, primary, weight, value);
	else
	{
		primary = _mot_alloc_node(MOT_TAG_ROOT, value->weight, 0, NULL);
		primary->parent = node;
		primary->payload.head = value;
		primary->red = SP_TRUE;
	    isMajor ? (node->major = primary) : (node->minor = primary);
	    _mot_fix_rbt_violations(primary, head);
	}

    return primary;
}

void motInsertTree(MOT_tree* tree, const SPchar* name, MOT_tree value)
{
    MOT_CHECK_INPUT(name);
	SP_ASSERT(value && value->tag != MOT_TAG_NULL, "Cannot insert empty tree");
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
		SP_WARNING("Tag \"MOT_TAG_ROOT\" and \"MOT_TAG_ARRAY\" are mutually exclusive");
		return;
	}

	if(scalar == MOT_TAG_NULL)
	{
		SP_WARNING("Expected type");
		return;
	}

	if(!(data && length > 0))
	{
		SP_WARNING("Cannot feed with empty array");
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

SPbuffer motWriteBinary(const MOT_tree tree)
{
	SPbuffer buffer = SP_BUFFER_INIT;
	if(!tree)
	{
		SP_WARNING("Cannot serialize empty tree");
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
	MOT_CHECKED_CALLOC(tree, 1, sizeof(struct MOT_node), return NULL);
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

MOT_tree motReadBinary(SPbuffer buffer)
{
    SPbuffer decompressed = _mot_decompress(buffer.data, buffer.length);
	const SPubyte** memory = (const SPubyte**) &decompressed.data;
	SPsize length = decompressed.length;
	
	return _mot_read_binary(memory, &length);
}

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic pop
#endif