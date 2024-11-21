#include "SP/sparse/mt3.h"
#include <errno.h>
#include <stdarg.h>
#include <zlib.h>
#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wformat"
#endif
#define MT3_CHECKED_CALLOC(ptr, n, size, on_error)		\
	do							\
	{							\
		if(!((ptr) = calloc(n, size)))			\
		{						\
			errno = MT3_STATUS_NO_MEMORY;		\
			on_error;				\
		}						\
	} while(0)

#define MT3_CHECK_INPUT(_name)					\
	SPhash hash = 0;					\
	do							\
	{							\
	    	if(!tree)\
	    	{                                           	\
	        	errno = MT3_STATUS_BAD_VALUE;		\
	       		return;                                 \
	    	}                                           	\
			else\
			{\
				if((*tree) && (*tree)->parent)\
				{\
					errno = MT3_STATUS_BAD_VALUE;\
					return;\
				}\
			}\
	    	if(!_name)                                  	\
	    	{                                           	\
	        	errno = MT3_STATUS_BAD_NAME;		\
	        	return;                         	\
		}                                       	\
		hash = _mt3_sdbm((_name));			\
		if(hash == 0)                           	\
		{                                       	\
		    errno = MT3_STATUS_BAD_NAME;    		\
		    return;                             	\
		}                                       	\
	} while(0)

#define MT3_READ_GENERIC(dst, n, scanner, fail)     		\
	do							\
	{							\
		if(*length < (n))				\
		{						\
			errno = MT3_STATUS_READ_ERROR;\
			fail;					\
		}						\
		*memory = scanner((dst), *memory, (n));		\
		*length -= n;					\
	} while(0)

#define MT3_FOR_EACH(node, cursor)\
	for((cursor) = (node); (cursor) != NULL; (cursor) = (cursor)->major)
		
#define MT3_MAX(a, b) ((a) > (b) ? (a) : (b))
#define ne2be _mt3_big_endian_to_native_endian
#define be2ne _mt3_big_endian_to_native_endian

struct MT3_node
{
	//written to disk:
	SPhash weight; // 8 bytes
	MT3_tag tag;   // 1 byte
	SPsize length; // 8 bytes

	// encoded in the 7th bit of the tag..
	// signals the node's color..
	SPbool red;

	union
	{
		SPbyte* data;

		// does not hold the root-node
		// but only major and minor branches..
		MT3_tree head;
	} payload; //length bytes

	// total bytes written = 17 + length bytes..
	struct MT3_node* parent;
	struct MT3_node* major;
	struct MT3_node* minor;
};

static MT3_tree _mt3_copy_tree(const MT3_tree tree);
static const char* _mt3_tag_to_str(MT3_tag tag);
static void _mt3_encode(const MT3_tree tree, SPbuffer* buffer, int level);
static void _mt3_insert_array(MT3_tree* tree, const SPchar* name, MT3_tag tag, SPsize length, const void* data);
static void _mt3_write_binary(const MT3_tree tree, SPbuffer* buffer, int level);
static SPchar* _mt3_init_string_array_buffer(SPsize* bCount, SPsize length, const SPchar** memory);
static MT3_tree _mt3_read_binary(const SPubyte** memory, SPsize* length);
static void _mt3_print_tree(const MT3_tree tree, int level);
static void _mt3_print_array(const MT3_array array, int level);
static void _mt3_fix_rbt_violations(MT3_node* node, MT3_tree* head);
static MT3_node* _mt3_rotate_left(MT3_node* n, MT3_tree* head);
static MT3_node* _mt3_rotate_right(MT3_node* n, MT3_tree* head);
static void _mt3_bst_delete_impl(MT3_node* n, MT3_tree* head, MT3_node** x, MT3_node** w, MT3_node** r);
static SPbool _mt3_is_major(const MT3_node* node);
static SPbool _mt3_is_root(const MT3_node* node);
static SPbool _mt3_fix_up_rbt(SPbool rBefore, MT3_node* r, MT3_node* x, MT3_node* w, MT3_tree* head);
static SPbool _mt3_transplant_rbt(MT3_node* x, MT3_node* w, MT3_tree* head);
static SPbool _mt3_transplant_proc_0(MT3_node* x);
static SPbool _mt3_transplant_proc_1(MT3_node* x, MT3_node* w, MT3_tree* head);
static SPbool _mt3_transplant_proc_2(MT3_node* x, MT3_node* w, MT3_tree* head);
static SPbool _mt3_transplant_proc_3(MT3_node* x, MT3_node* w, MT3_tree* head);
static SPbool _mt3_transplant_proc_4(MT3_node* x, MT3_node* w, MT3_tree* head);

static int _mt3_is_little_endian()
{
	SPuint16 t = 0x0001;
	SPchar c[2];
	memcpy(c, &t, sizeof t);
	return c[0];
}

static void* _mt3_swap_bytes(void* s, SPsize length)
{
	for(SPchar *b = s, *e = b + length - 1; b < e; b++, e--)
	{
		SPchar t = *b;
		*b = *e;
		*e = t;
	}
	return s;
}

static void* _mt3_big_endian_to_native_endian(void* s, size_t len)
{
	return _mt3_is_little_endian() ? _mt3_swap_bytes(s, len) : s;
}

static const void* _mt3_memcpy(void* dst, const void* src, SPsize n)
{
	memcpy(dst, src, n);
	return (const SPchar*) src + n;
}

static const void* _mt3_swapped_memcpy(void* dst, const void* src, SPsize n)
{
	const void* ret = _mt3_memcpy(dst, src, n);
	return ne2be(dst, n), ret;
}

static SPbuffer _mt3_compress(const void* memory, SPsize length)
{
	const SPsize CHUNK_SIZE = 4096;
	SPbuffer buffer = SP_BUFFER_INIT;

	z_stream stream =
	{
		.zalloc   = Z_NULL,
		.zfree	  = Z_NULL,
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
	    errno = MT3_STATUS_WRITE_ERROR;
		return SP_BUFFER_INIT;
	}

    	if(stream.avail_in != length)
    	{
        	errno = MT3_STATUS_WRITE_ERROR;
        	return buffer;
    	}

	do
	{
		if(spBufferReserve(&buffer, buffer.length + CHUNK_SIZE) != MT3_STATUS_OK)
		{
			errno = MT3_STATUS_NO_MEMORY;
			spBufferFree(&buffer);
			return SP_BUFFER_INIT;
		}

		stream.next_out = buffer.data + buffer.length;
		stream.avail_out = CHUNK_SIZE;

		if(deflate(&stream, Z_FINISH) == Z_STREAM_ERROR)
		{
			errno = MT3_STATUS_WRITE_ERROR;
			spBufferFree(&buffer);
			return SP_BUFFER_INIT;
		}

		buffer.length += CHUNK_SIZE - stream.avail_out;
	} while(!stream.avail_out);

	deflateEnd(&stream);
	return buffer;
}

static SPbuffer _mt3_decompress(const void* memory, SPsize length)
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
	    errno = MT3_STATUS_READ_ERROR;
		return SP_BUFFER_INIT;
	}

	SPint zlib_ret;
	do
	{
		if(spBufferReserve(&buffer, buffer.length + CHUNK_SIZE))
		{
		    errno = MT3_STATUS_NO_MEMORY;
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
				errno = MT3_STATUS_READ_ERROR;
				spBufferFree(&buffer);
				return SP_BUFFER_INIT;
			}
			default:
				buffer.length += CHUNK_SIZE - stream.avail_out;
		}
	} while(!stream.avail_out);

	if(zlib_ret != Z_STREAM_END)
	{
		errno = MT3_STATUS_READ_ERROR;
		return SP_BUFFER_INIT;
	}

	inflateEnd(&stream);
	return buffer;
}

static MT3_node* _mt3_alloc_node(MT3_tag tag, SPhash name, SPsize length, const void* value)
{
	MT3_node* node = NULL;
	MT3_CHECKED_CALLOC(node, 1, sizeof(MT3_node), return NULL);
	node->tag = tag;
	node->weight = name;
	node->length = length;
	node->major = node->minor = NULL;
	node->parent = NULL;
	node->red = SP_FALSE;
	if(tag != MT3_TAG_NULL)
	{
		if(!(tag & MT3_TAG_ROOT) && tag != MT3_TAG_ARRAY)
		{
			if(value && length > 0)
			{
				MT3_CHECKED_CALLOC(node->payload.data, node->length, sizeof(SPbyte), return NULL);
				memcpy(node->payload.data, (const SPbyte*) value, node->length * sizeof(SPbyte));

				if(tag == MT3_TAG_STRING)
					node->payload.data[node->length - 1] = 0;
			}
		}
		else
		{
			if(value)
			{
				node->payload.head = (tag == MT3_TAG_ROOT) ? mt3_CopyTree((const MT3_tree) value) : mt3_CopyArray((const MT3_array) value);
				//node->payload.head = (MT3_tree) value;
			}
			else
			{
				node->payload.head = _mt3_alloc_node(MT3_TAG_NULL, name, 0LL, NULL);
			}
		}
	}
	return node;
}

// very dangerous, can cause disruption!!
static void _mt3_delete_node(MT3_node* n)
{
	if(n)
	{
		if(n->tag != MT3_TAG_ARRAY && n->tag != MT3_TAG_ROOT_ARRAY)
		{
			if(n->tag != MT3_TAG_ROOT)
			{
				free(n->payload.data);
			}
			else
			{
				mt3_FreeTree(&n->payload.head);
			}
		}
		else
		{
			mt3_FreeArray(&n->payload.head);
		}
		free(n);
	}
}

static SPsize _mt3_length_of(MT3_tag tag)
{
	switch(tag)
	{
		case MT3_TAG_BYTE: return sizeof(SPbyte);
		case MT3_TAG_SHORT: return sizeof(SPshort);
		case MT3_TAG_INT: return sizeof(SPint);
		case MT3_TAG_LONG: return sizeof(SPlong);
		case MT3_TAG_FLOAT: return sizeof(SPfloat);
		case MT3_TAG_DOUBLE: return sizeof(SPdouble);
		case MT3_TAG_NULL: break;
	}
	return 0;
}

static SPhash _mt3_sdbm_impl(const SPchar *str)
{
	SPhash hash = 0;
	int c;
	while ((c = *str++))
		hash = c + (hash << 6) + (hash << 16) - hash; // hash * 65599 + c

	return hash;
}

static SPhash _mt3_sdbm(const SPchar* str)
{
	SPhash hash = _mt3_sdbm_impl(str);
	SPubyte buffer[4];

	for(SPsize i = 0; i < 8; i++)
	{
		buffer[i] = (hash >> (i * 8)) & 0xFF;
	}
	SPhash output = 0;
	memcpy(&output, buffer, sizeof(SPbyte) * 4);
	return output;
}

static SPchar* _mt3_init_string_array_buffer( SPsize* bCount, SPsize length, const SPchar** strings)
{
	SPchar* memory = NULL;
	if(strings)
	{
		SPsize byteCount = length * (sizeof(SPlong) + 1) + 1;
		for(SPsize i = 0; i < length; i++)
		{
			const SPchar* string = strings[i];
			SP_ASSERT(string, "Cannot copy from NULL-string");
			byteCount += strlen(string);
		}

		
		MT3_CHECKED_CALLOC(memory, byteCount, sizeof(SPbyte), return NULL);

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
		if(bCount)
			*bCount = byteCount;
	}
	return memory;
}

void _mt3_print_binary(const unsigned char *byteArray, size_t size, int level)
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

MT3_tree mt3_AllocObject()
{
	MT3_node* root = NULL;
	MT3_CHECKED_CALLOC(root, 1, sizeof(MT3_node), return NULL);

	root->length = 0LL;
	root->weight = 0LL;
	root->tag = MT3_TAG_NULL;
	root->major = root->minor = NULL;
	return root;
}

MT3_tree mt3_AllocArray()
{
	MT3_tree list = mt3_AllocObject();
	list->red = SP_TRUE;
	return list;
}

MT3_tree mt3_CopyTree(const MT3_tree n)
{
    MT3_tree tree = NULL;
    if(n)
    {
        tree = mt3_AllocObject();
        if(!tree)
        {
            errno = MT3_STATUS_NO_MEMORY;
            return NULL;
        }

        tree->red = n->red;
        tree->weight = n->weight;
        tree->tag = n->tag;
        tree->length = n->length;

        if(n->tag != MT3_TAG_ROOT)
        {
            tree->payload.data = malloc(n->length);
            if(!tree->payload.data)
            {
                errno = MT3_STATUS_NO_MEMORY;
                free(tree);
                return NULL;
            }

            memcpy(tree->payload.data, n->payload.data, n->length);
        }
        else
        {
            tree->payload.head = mt3_CopyTree(n->payload.head);
        }

        tree->major = mt3_CopyTree(n->major);
        tree->minor = mt3_CopyTree(n->minor);

        if(tree->major)
           tree->major->parent = tree;

        if(tree->minor)
           tree->minor->parent = tree;
    }
    return tree;
}

MT3_array mt3_CopyArray(const MT3_array n)
{
	MT3_array array = NULL;
	if(n)
	{
		if(n->red && !n->parent)
		{
			array = mt3_AllocArray();
			MT3_array dst_cursor = array;
			MT3_array src_cursor = NULL;
			
			MT3_FOR_EACH(n, src_cursor)
			{
				switch(src_cursor->tag)
				{
					case MT3_TAG_ROOT:
					{
						dst_cursor->tag = MT3_TAG_ROOT;
						dst_cursor->payload.head = mt3_CopyTree(src_cursor->payload.head);
						break;
					}
					/*
					case MT3_TAG_ARRAY:
					{
						dst_cursor->tag = MT3_TAG_ARRAY;
						dst_cursor->payload.head = mt3_CopyArray(src_cursor->payload.head);
						break;
					}
					default:
					{
						if(src_cursor->tag & MT3_TAG_ARRAY)
						{
							dst_cursor->tag = src_cursor->tag;
							dst_cursor->length = src_cursor->length;
							MT3_CHECKED_CALLOC(dst_cursor->payload.data, src_cursor->length, sizeof(SPbyte), return NULL);
							memcpy(dst_cursor->payload.data, src_cursor->payload.data, sizeof(SPbyte) * src_cursor->length);
							
						}
						break;
					}
					*/
				}
				
				dst_cursor->major = mt3_AllocArray();
				MT3_array ptr = dst_cursor;
				dst_cursor = dst_cursor->major;
				dst_cursor->minor = ptr;
			}
			
			dst_cursor->minor->major = NULL;
			free(dst_cursor);
		}
	}
	return array;
}

void _mt3_free_tree_impl(MT3_tree tree)
{
	if(tree)
	{
		if(tree->tag != MT3_TAG_ROOT_ARRAY && tree->tag != MT3_TAG_ARRAY)
		{
			if(tree->tag != MT3_TAG_ROOT)
			{
				free(tree->payload.data);
			}
			else
			{
				_mt3_free_tree_impl(tree->payload.head);
			}
		}
		else
		{
			mt3_FreeArray(&tree->payload.head);
		}
		
		_mt3_free_tree_impl(tree->major);
		_mt3_free_tree_impl(tree->minor);
		free(tree);
	}
}

void _mt3_free_array_impl(MT3_array array)
{
	if(array && array->payload.head)
	{
		MT3_array cursor = NULL;
		for(cursor = array; cursor != NULL;)
		{
			//free the payload..
			if(cursor->tag == MT3_TAG_ROOT)
			{
				//tree-array..
				_mt3_free_tree_impl(cursor->payload.head);
				cursor->payload.head = NULL;
			}
			else if(cursor->tag == MT3_TAG_ARRAY)
			{
				//multi-array..
				_mt3_free_array_impl(cursor->payload.head);
				cursor->payload.head = NULL;
			}
			else if(array->tag & MT3_TAG_ARRAY)
			{
				//plain array..
				free(cursor->payload.data);
				cursor->payload.data = NULL;
			}
			
			MT3_array n = cursor;
			cursor = cursor->major;
			free(n);
		}
		array->payload.head = NULL;
	}
}

void mt3_FreeArray(MT3_array* array)
{
	_mt3_free_array_impl(*array);
	array = NULL;
}

void mt3_FreeTree(MT3_tree* tree)
{
    _mt3_free_tree_impl(*tree);
    *tree = NULL;
}

static MT3_tree _mt3_search_for(MT3_tree tree, SPhash hash)
{
	MT3_tree node = NULL;
	if(tree)
	{
		if(tree->weight == hash)
			return tree;
		
		if(hash > tree->weight)
			node = _mt3_search_for(tree->major, hash);
		else
			node = _mt3_search_for(tree->minor, hash);
	}
	return node;
}

MT3_node* mt3_Search(const MT3_tree tree, const char* name)
{
	SPhash hash = _mt3_sdbm(name);
	return _mt3_search_for(tree, hash);
}

static MT3_tree _mt3_insert_data(MT3_tree* head, MT3_tree node, SPhash weight, MT3_tag tag, SPsize length, const void* value)
{
	if(!node)
	{
		*head = _mt3_alloc_node(tag, weight, length, value);
		return *head;
	}

	if(weight == node->weight)
	{
		errno = MT3_STATUS_BAD_NAME;
		return NULL;
	}

	if(node->weight == 0 || node->tag == MT3_TAG_NULL)
	{
		SP_ASSERT(node->length == 0LL, "Empty tree cannot have length defined (%lld)", node->length);
		SP_ASSERT(!node->payload.data, "Empty tree cannot have data");
		SP_ASSERT(!node->major && !node->minor, "Empty tree cannot have sub-trees");
		SP_ASSERT(!node->parent, "Empty tree cannot have a parent");
		SP_ASSERT(!node->red, "Empty tree must be black-coded");

		*head = _mt3_alloc_node(tag, weight, length, value);
		return *head;
	}

	SPbool maj = (weight > node->weight);
	MT3_tree primary  = maj ? node->major : node->minor;

	if(primary)
	{
		_mt3_insert_data(head, primary, weight, tag, length, value);
	}
	else
	{
		primary = _mt3_alloc_node(tag, weight, length, value);
		primary->parent = node;
		primary->red = SP_TRUE;
		maj ? (node->major = primary) : (node->minor = primary);
		_mt3_fix_rbt_violations(primary, head);
	}
	return primary;
}

void mt3_InsertTree(MT3_tree* tree, const SPchar* name, MT3_tree value)
{
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_ROOT, 0LL, value);
}

void mt3_InsertByte(MT3_tree* tree, const SPchar* name, SPbyte value)
{
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_BYTE, sizeof(SPbyte), &value);
}

void mt3_InsertShort(MT3_tree* tree, const SPchar* name, SPshort value)
{
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_SHORT, sizeof(SPshort), &value);
}

void mt3_InsertInt(MT3_tree* tree, const SPchar* name, SPint value)
{
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_INT, sizeof(SPint), &value);
}

void mt3_InsertLong(MT3_tree* tree, const SPchar* name, SPlong value)
{
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_LONG, sizeof(SPlong), &value);
}

void mt3_InsertFloat(MT3_tree* tree, const SPchar* name, SPfloat value)
{
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_FLOAT, sizeof(SPfloat), &value);
}

void mt3_InsertDouble(MT3_tree* tree, const SPchar* name, SPdouble value)
{
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_DOUBLE, sizeof(SPdouble), &value);
}

void mt3_InsertString(MT3_tree* tree, const SPchar* name, const SPchar* value)
{
	if(value)
	{
		MT3_CHECK_INPUT(name);
		_mt3_insert_data(tree, *tree, hash, MT3_TAG_STRING, strlen(value) + 1, value);
	}
}

void _mt3_insert_array(MT3_tree* tree, const SPchar* name, MT3_tag tag, SPsize length, const void* data)
{
	MT3_tag scalar = tag & ~MT3_TAG_ARRAY;
    if(scalar > MT3_TAG_STRING)
    {
        errno = MT3_STATUS_BAD_TAG;
        return;
    }

	if(scalar == MT3_TAG_NULL)
	{
		errno = MT3_STATUS_BAD_TAG;
		return;
	}

	if(!(data && length > 0))
	{
		errno = MT3_STATUS_BAD_VALUE;
		return;
	}

	MT3_CHECK_INPUT(name);
	if(scalar != MT3_TAG_STRING)
	{
		_mt3_insert_data(tree, *tree, hash, tag | MT3_TAG_ARRAY, length * _mt3_length_of(scalar), data);
	}
	else
	{
		SPsize byteCount = 0; 
		const SPchar** strings = (const SPchar**) data;
		
		SPchar* memory = _mt3_init_string_array_buffer(&byteCount, length, strings);
		_mt3_insert_data(tree, *tree, hash, tag | MT3_TAG_ARRAY, byteCount, memory);
		free(memory);
	}
}

void mt3_InsertByteArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPbyte* values)
{
    _mt3_insert_array(tree, name, MT3_TAG_BYTE, length, values);
}

void mt3_InsertShortArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPshort* values)
{
    _mt3_insert_array(tree, name, MT3_TAG_SHORT, length, values);
}

void mt3_InsertIntArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPint* values)
{
    _mt3_insert_array(tree, name, MT3_TAG_INT, length, values);
}

void mt3_InsertLongArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPlong* values)
{
    _mt3_insert_array(tree, name, MT3_TAG_LONG, length, values);
}

void mt3_InsertFloatArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPfloat* values)
{
    _mt3_insert_array(tree, name, MT3_TAG_FLOAT, length, values);
}

void mt3_InsertDoubleArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPdouble* values)
{
    _mt3_insert_array(tree, name, MT3_TAG_DOUBLE, length, values);
}

void mt3_InsertStringArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPchar** values)
{
    _mt3_insert_array(tree, name, MT3_TAG_STRING, length, values);
}

void mt3_InsertArray(MT3_tree* tree, const SPchar* name, MT3_array list)
{
	if(list)
	{
		MT3_CHECK_INPUT(name);
		_mt3_insert_data(tree, *tree, hash, list->tag | MT3_TAG_ARRAY, list->length, list);
	}
}


void mt3_ArrayInsertTree(MT3_array* _array, MT3_tree value)
{
	if(_array)
	{
		MT3_array array = *_array;
		if(!array)
		{
			*_array = mt3_AllocArray();
			array = *_array;
			array->tag = MT3_TAG_ROOT;
			array->payload.head = value;
			array->length++;
		}
		else
		{
			if(array->red && !array->parent && array->tag == MT3_TAG_ROOT)
			{
				MT3_array cursor = array;
				while(cursor->major)
					cursor = cursor->major;
				
				cursor->major = mt3_AllocArray();
				cursor->major->tag = MT3_TAG_ROOT;
				cursor->major->red = SP_TRUE;
				cursor->major->minor = cursor;
				cursor->major->payload.head = value;
				array->length++;
			}
		}
	}
}


/*
SP_API void mt3_ArrayInsertByteArray(MT3_array array, const SPchar* name, SPsize length, const SPbyte* values);
SP_API void mt3_ArrayInsertShortArray(MT3_array array, const SPchar* name, SPsize length, const SPshort* values);
SP_API void mt3_ArrayInsertIntArray(MT3_array array, const SPchar* name, SPsize length, const SPint* values);
SP_API void mt3_ArrayInsertLongArray(MT3_array array, const SPchar* name, SPsize length, const SPlong* values);
SP_API void mt3_ArrayInsertFloatArray(MT3_array array, const SPchar* name, SPsize length, const SPfloat* values);
SP_API void mt3_ArrayInsertDoubleArray(MT3_array array, const SPchar* name, SPsize length, const SPdouble* values);
SP_API void mt3_ArrayInsertStringArray(MT3_array array, const SPchar* name, SPsize length, const SPchar** values);
SP_API void mt3_ArrayInsertArray(MT3_array array, const SPchar* name, MT3_array list);
*/

#define MT3_SET_VALUE(_tag, _value)\
	do\
	{\
		MT3_node* n = mt3_Search(tree, name);\
		SP_ASSERT(n, "Failed to write node %s", name);\
		if(n && n->tag == (_tag))\
		{\
			memcpy(n->payload.data, (_value), _mt3_length_of((_tag)));\
		}\
	} while(0);
	
#define MT3_GET_VALUE(_tag)\
	do\
	{\
		MT3_node* n = mt3_Search(tree, name);\
		SP_ASSERT(n, "Failed to read node %s", name);\
		if(n && n->tag == (_tag))\
		{\
			memcpy(&ret, n->payload.data, _mt3_length_of((_tag)));\
		}\
	} while(0);
	
void mt3_SetByte(MT3_tree tree, const char* name, SPbyte value)
{
	MT3_SET_VALUE(MT3_TAG_BYTE, &value);
}

void mt3_SetShort(MT3_tree tree, const char* name, SPshort value)
{
	MT3_SET_VALUE(MT3_TAG_SHORT, &value);
}

void mt3_SetInt(MT3_tree tree, const char* name, SPint value)
{
	MT3_SET_VALUE(MT3_TAG_INT, &value);
}

void mt3_SetLong(MT3_tree tree, const char* name, SPlong value)
{
	MT3_SET_VALUE(MT3_TAG_LONG, &value);
}

void mt3_SetFloat(MT3_tree tree, const char* name, SPfloat value)
{
	MT3_SET_VALUE(MT3_TAG_FLOAT, &value);
}

void mt3_SetDouble(MT3_tree tree, const char* name, SPdouble value)
{
	MT3_SET_VALUE(MT3_TAG_DOUBLE, &value);
}

void mt3_SetString(MT3_tree tree, const char* name, const SPchar* value)
{
	MT3_node* n = mt3_Search(tree, name);
	if(n && n-> tag == MT3_TAG_STRING)
	{
		SP_ASSERT(n->payload.data, "Expected data");
		SPsize length = strlen(value) + 1;
		if(length != n->length)
		{
			SPbyte* buf = realloc(n->payload.data, length * sizeof(SPchar));
			if(!buf)
			{
				errno = MT3_STATUS_NO_MEMORY;
				return;
			}
			n->payload.data = buf;
			memcpy(n->payload.data, value, length - 1);
			n->payload.data[length - 1] = 0;
			n->length = length;
		}
	}
}

SPbyte mt3_GetByte(const MT3_tree tree, const SPchar* name)
{
	SPbyte ret = 0;
	MT3_GET_VALUE(MT3_TAG_BYTE);
	return ret;
}

SPshort mt3_GetShort(const MT3_tree tree, const SPchar* name)
{
	SPshort ret = 0;
	MT3_GET_VALUE(MT3_TAG_SHORT);
	return ret;
}

SPint mt3_GetInt(const MT3_tree tree, const SPchar* name)
{
	SPint ret = 0;
	MT3_GET_VALUE(MT3_TAG_INT);
	return ret;
}

SPlong mt3_GetLong(const MT3_tree tree, const SPchar* name)
{
	SPlong ret = 0;
	MT3_GET_VALUE(MT3_TAG_LONG);
	return ret;
}

SPfloat mt3_GetFloat(const MT3_tree tree, const SPchar* name)
{
	SPfloat ret = 0.f;
	MT3_GET_VALUE(MT3_TAG_FLOAT);
	return ret;
}

SPdouble mt3_GetDouble(const MT3_tree tree, const SPchar* name)
{
	SPdouble ret = 0.0;
	MT3_GET_VALUE(MT3_TAG_DOUBLE);
	return ret;
}

const SPchar* mt3_GetString(const MT3_tree tree, const SPchar* name)
{
	MT3_node* n = mt3_Search(tree, name);
	return (n && n->tag == MT3_TAG_STRING) ? n->payload.data : NULL;
}

MT3_tree* mt3_GetTree(const MT3_tree tree, const SPchar* name)
{
	MT3_node* n = mt3_Search(tree, name);
	return (n && n->tag == MT3_TAG_ROOT) ? &n->payload.head : NULL;
}

void mt3_Collect(const MT3_tree tree, SPbuffer* buffer)
{
	if(tree)
	{
		if(tree->minor)
			mt3_Collect(tree, buffer);
		if(tree->major)
			mt3_Collect(tree, buffer);
		
		if(tree->tag == MT3_TAG_ROOT)
		{
			spBufferAppend(buffer, (const SPubyte*) &tree->payload.head, sizeof(MT3_tree*));
		}
	}
}

//returns all root nodes..
MT3_tree** mt3_AsList(const MT3_tree tree)
{
	SPbuffer buffer = SP_BUFFER_INIT;
	mt3_Collect(tree, &buffer);
	return (MT3_tree**) buffer.data;
}

void* mt3_AllocChunk(SPsize size)
{
	SPbyte* ptr = NULL;
	MT3_CHECKED_CALLOC(ptr, size, sizeof(SPbyte), ;);
	return ptr;
}

/*<==========================================================>*
 *  debug
 *<==========================================================>*/

const char* _mt3_tag_to_str(MT3_tag tag)
{
	if(tag & MT3_TAG_ARRAY)
	{
		MT3_tag scalar = tag & ~MT3_TAG_ARRAY;
		switch(scalar)
		{
			case MT3_TAG_NULL: return "multi-array";
			case MT3_TAG_BYTE: return "byte-array";
			case MT3_TAG_SHORT: return "short-array";
			case MT3_TAG_INT: return "int-array";
			case MT3_TAG_LONG: return "long-array";
			case MT3_TAG_FLOAT: return "float-array";
			case MT3_TAG_DOUBLE: return "double-array";
			case MT3_TAG_STRING: return "string-array";
			case MT3_TAG_ROOT: return "tree-array";
		}
	}
	else
	{
		switch(tag)
		{
			case MT3_TAG_BYTE: return "byte";
			case MT3_TAG_SHORT: return "short";
			case MT3_TAG_INT: return "int";
			case MT3_TAG_LONG: return "long";
			case MT3_TAG_FLOAT: return "float";
			case MT3_TAG_DOUBLE: return "double";
			case MT3_TAG_STRING: return "string";
			case MT3_TAG_ARRAY: return "array";
			case MT3_TAG_ROOT: return "tree";
		}
	}
	return "null";
}

void _mt3_print(const MT3_tree tree, int level)
{
	if(tree)
	{
		SPchar color = tree->red ? 'R' : 'B';
		SPchar rank = _mt3_is_root(tree) ? '~' : (_mt3_is_major(tree) ? '+' : '-');
		
		switch(tree->tag)
		{
			case MT3_TAG_NULL:
			{
				break;
			}

			case MT3_TAG_ROOT:
			{
				printf("(%c%c) object (%lld):\n", color, rank, tree->weight);
				_mt3_print_tree(tree->payload.head, level + 1);
				break;
			}
			case MT3_TAG_BYTE:
			case MT3_TAG_SHORT:
			case MT3_TAG_INT:
			case MT3_TAG_LONG:
			{
					SPbyte* buffer = tree->payload.data;
					SP_ASSERT(buffer, "Node has invalid data");

					SPlong value = 0;
					memcpy(&value, buffer, tree->length * sizeof(SPbyte));

					printf("(%c%c) %s (%lld): %ld\n", color, rank, _mt3_tag_to_str(tree->tag), tree->weight, value);

					break;
			}
			
			case MT3_TAG_FLOAT:
			case MT3_TAG_DOUBLE:
			{
				SPbyte* buffer = tree->payload.data;
				SP_ASSERT(buffer, "Node has invalid data");
				union
				{
					float _f;
					double _d;
				} value;
				memcpy(((tree->tag == MT3_TAG_FLOAT) ? (void*)&value._f : (void*)&value._d), buffer, tree->length * sizeof(SPbyte));
				printf("(%c%c) %s (%lld): %f\n", color, rank, _mt3_tag_to_str(tree->tag), tree->weight, (tree->tag == MT3_TAG_FLOAT) ? value._f : value._d);
				break;
			}
			
			case MT3_TAG_STRING:
			{
				SPbyte* buffer = tree->payload.data;
				SP_ASSERT(buffer, "Node has invalid data");
				printf("(%c%c) string (%lld): \"%s\"\n", color, rank, tree->weight, buffer);
				break;
			}
			
			case MT3_TAG_BYTE_ARRAY:
			case MT3_TAG_SHORT_ARRAY:
			case MT3_TAG_INT_ARRAY:
			case MT3_TAG_LONG_ARRAY:
			{
				printf("(%c%c) %s (%lld): ", color, rank, _mt3_tag_to_str(tree->tag), tree->length, tree->weight);
				SPsize stride = _mt3_length_of(tree->tag & ~MT3_TAG_ARRAY);
				for(SPsize i = 0; i < tree->length; i += stride)
				{
					SPlong l = 0;
					memcpy(&l, tree->payload.data + i, stride * sizeof(SPbyte));
					printf("%ld ", l);
				}
				printf("\n");
				break;
			}
			
			case MT3_TAG_FLOAT_ARRAY:
			case MT3_TAG_DOUBLE_ARRAY:
			{
				printf("(%c%c) %s (%lld): ", color, rank, _mt3_tag_to_str(tree->tag), tree->length, tree->weight);
				SPsize stride = _mt3_length_of(tree->tag & ~MT3_TAG_ARRAY);
				union
				{
					float _f;
					double _d;
				} value;
				for(SPsize i = 0; i < tree->length; i += stride)
				{
					(tree->tag == MT3_TAG_FLOAT_ARRAY) ? (value._f = 0.f) : (value._d = 0.0);
					memcpy(((tree->tag == MT3_TAG_FLOAT_ARRAY) ? (void*)&value._f : (void*)&value._d), tree->payload.data + i, stride);
					printf("%f ", ((tree->tag == MT3_TAG_FLOAT_ARRAY) ? value._f : value._d));
				}
				printf("\n");
				break;
			}
			
			case MT3_TAG_STRING_ARRAY:
			{
				printf("(%c%c) %s (%lld): ", color, rank, _mt3_tag_to_str(tree->tag), tree->length, tree->weight);
				SPsize i = 0;
				while(i < tree->length - 1)
				{
					SPlong size = 0;
					memcpy(&size, tree->payload.data + i, sizeof(SPlong));
					if(size == 0)
						break;

					i += sizeof(SPlong);
					SPchar* str = NULL;
					MT3_CHECKED_CALLOC(str, size, sizeof(SPchar), return);
					memcpy(str, tree->payload.data + i, size);
					printf("\"%s\" ", str);
					free(str);
					i += size;
				}
				printf("\n");
				break;
			}
			
			case MT3_TAG_ROOT_ARRAY:
			{
				
				printf("(%c%c) %s (%lld elements) (%lld):\n", color, rank, _mt3_tag_to_str(tree->tag), tree->length, tree->weight);
				_mt3_print_array(tree->payload.head, level + 1);
				break;
			}
			/*
			case MT3_TAG_ARRAY:
			{
				printf("(%c%c) %s (%lld): ", color, rank, _mt3_tag_to_str(tree->tag), tree->length, tree->weight);
				MT3_array cursor = NULL;
				MT3_FOR_EACH(tree->payload.head, cursor)
				{
					_mt3_print(cursor, level + 1);
				}
				break;
			}
			*/
		}
	}
}

void _mt3_print_array(const MT3_array array, int level)
{
	if(array)
	{
		MT3_array cursor = NULL;
		MT3_FOR_EACH(array, cursor)
		{
			switch(cursor->tag)
			{
				case MT3_TAG_ROOT:
				{
					_mt3_print_tree(cursor->payload.head, level);
					break;
				}
				/*
				case MT3_TAG_ARRAY:
				{
					_mt3_print_array(cursor->payload.head, level + 1);
					break;
				}
				
				case MT3_TAG_BYTE_ARRAY:
				case MT3_TAG_SHORT_ARRAY:
				case MT3_TAG_INT_ARRAY:
				case MT3_TAG_LONG_ARRAY:
				{
					printf("%s: ", _mt3_tag_to_str(array->tag));
					SPsize stride = _mt3_length_of(array->tag & ~MT3_TAG_ARRAY);
					for(SPsize i = 0; i < array->length; i += stride)
					{
						SPlong l = 0;
						memcpy(&l, array->payload.data + i, stride * sizeof(SPbyte));
						printf("%ld ", l);
					}
					printf("\n");
					break;
				}
				
				case MT3_TAG_FLOAT_ARRAY:
				case MT3_TAG_DOUBLE_ARRAY:
				{
					printf("%s: ", _mt3_tag_to_str(array->tag));
					SPsize stride = _mt3_length_of(array->tag & ~MT3_TAG_ARRAY);
					union
					{
						float _f;
						double _d;
					} value;
					for(SPsize i = 0; i < array->length; i += stride)
					{
						(array->tag == MT3_TAG_FLOAT_ARRAY) ? (value._f = 0.f) : (value._d = 0.0);
						memcpy(((array->tag == MT3_TAG_FLOAT_ARRAY) ? (void*)&value._f : (void*)&value._d), array->payload.data + i, stride);
						printf("%f ", ((array->tag == MT3_TAG_FLOAT_ARRAY) ? value._f : value._d));
					}
					printf("\n");
					break;
				}
				
				case MT3_TAG_STRING_ARRAY:
				{
					printf("%s: ", _mt3_tag_to_str(tree->tag));
					SPsize i = 0;
					while(i < tree->length - 1)
					{
						SPlong size = 0;
						memcpy(&size, tree->payload.data + i, sizeof(SPlong));
						if(size == 0)
							break;

						i += sizeof(SPlong);
						SPchar* str = NULL;
						MT3_CHECKED_CALLOC(str, size, sizeof(SPchar), return);
						memcpy(str, tree->payload.data + i, size);
						printf("\"%s\" ", str);
						free(str);
						i += size;
					}
					printf("\n");
					break;
				}
				*/
			}
		}
	}
}

void _mt3_print_tree(const MT3_tree tree, int level)
{
	if(tree)
	{
		for(int i = 0; i < level; i++)
		printf("\t");
		
		_mt3_print(tree, level);
		_mt3_print_tree(tree->major, level + 1);
		_mt3_print_tree(tree->minor, level + 1);
	}
}

void mt3_PrintArray(const MT3_array array)
{
	if(array)
	{
		_mt3_print_array(array, 0);
	}
}
void mt3_PrintTree(const MT3_tree tree)
{
	if(tree && !tree->parent)
	{
		_mt3_print_tree(tree, 0);
		printf("\n~ ... Root\n");
		printf("+ ... Major\n");
		printf("- ... Minor\n");
		printf("B ... Black\n");
		printf("R ... Red\n\n");
		printf("\n");
	}
}


#define MT3_PRINT_OUTPUT_DEBUG
static void _mt3_write_bytes(SPbuffer* buffer, const SPubyte* src, SPsize amount, int level, SPbool toNativeEndian)
{
	SPbyte* chunk = mt3_AllocChunk(amount);
	(toNativeEndian) ? _mt3_swapped_memcpy(chunk, src, amount) : _mt3_memcpy(chunk, src, amount);
	if(spBufferAppend(buffer, chunk, amount) != MT3_STATUS_OK)
	{
		free(chunk);
		errno = MT3_STATUS_WRITE_ERROR;
		return;
	}
	
	
#ifdef MT3_PRINT_OUTPUT_DEBUG
	_mt3_print_binary(src, amount, level);
#endif
	free(chunk);
}

#ifdef MT3_PRINT_OUTPUT_DEBUG
#define _mt3_print_indent(ntabs, msg, ...)\
	do\
	{\
		for(int i = 0; i < ntabs; i++)\
			printf("\t");\
		printf((msg), ##__VA_ARGS__);\
		printf("\n");\
	} while(0)
#else
#define _mt3_print_indent(ntabs, msg, ...)
#endif

static void _mt3_encode_array(const MT3_array array, SPbuffer* buffer, int level)
{	
	MT3_array cursor = NULL;
	SPsize i = 0;
	for(cursor = array; cursor != NULL; cursor = cursor->major)
	{
		SPuint8 tag = array->tag | ((array->red & 1) << 6);
		_mt3_print_indent(level, "%lld. element, tag: (%s)", i++, _mt3_tag_to_str(array->tag));
		_mt3_write_bytes(buffer, (const SPubyte*) &tag, sizeof(SPbyte), level, SP_FALSE);
		switch(cursor->tag)
		{
			case MT3_TAG_ROOT:
			{
				_mt3_write_binary(cursor->payload.head, buffer, level + 1);
				break;
			}
			/*
			case MT3_TAG_ARRAY:
			{
				_mt3_encode_array(const MT3_array array, SPbuffer* buffer, int level)
				break;
			}
			
			case MT3_TAG_BYTE_ARRAY:
			case MT3_TAG_SHORT_ARRAY:
			case MT3_TAG_INT_ARRAY:
			case MT3_TAG_LONG_ARRAY:
			case MT3_TAG_FLOAT_ARRAY:
			case MT3_TAG_DOUBLE_ARRAY:
			{
				_mt3_print_indent(level, "length: %lld", array->length);
				_mt3_write_bytes(buffer, (const SPubyte*) &tree->length, sizeof(SPsize), level, SP_TRUE);
				
				SP_ASSERT(tree->payload.data, "Node %lld has invalid data to write", tree->weight);
				_mt3_print_indent(level, "data: (%lld byte(s))", tree->length);
				SPsize stride = _mt3_length_of(tree->tag & ~MT3_TAG_ARRAY);
				for(SPsize i = 0; i < tree->length / stride; i++)
					_mt3_write_bytes(buffer, (const SPubyte*)tree->payload.data + stride * i, stride, level, SP_TRUE);
				break;
			}
			
			case MT3_TAG_STRING_ARRAY:
			{
				_mt3_print_indent(level, "length: %lld", tree->length);
				_mt3_write_bytes(buffer, (const SPubyte*) &tree->length, sizeof(SPsize), level, SP_TRUE);
				
				SP_ASSERT(tree->payload.data, "Node %lld has invalid data to write", tree->weight);
				_mt3_print_indent(level, "data: (%lld byte(s))", tree->length);
				SPsize i = 0;
				while(i < tree->length - 1)
				{
					SPlong length = 0;
					memcpy(&length, tree->payload.data + i, sizeof(SPlong));
					_mt3_write_bytes(buffer, (const SPubyte*) &length, sizeof(SPlong), level, SP_TRUE);
					i += sizeof(SPlong);
					_mt3_write_bytes(buffer, (const SPubyte*) tree->payload.data + i, length, level, SP_FALSE);
					i += length;
				}
				SPubyte zero = 0;
				_mt3_write_bytes(buffer, (const SPubyte*) &zero, sizeof(SPbyte), level, SP_FALSE);			
				break;
			}
			*/
		}
	}
	_mt3_print_indent(level, "(list end)");
	SPbyte zero = 0;
	_mt3_write_bytes(buffer, &zero, sizeof(SPbyte), level, SP_FALSE);	
}

static void _mt3_encode(const MT3_tree tree, SPbuffer* buffer, int level)
{	
	if(tree)
	{
		switch(tree->tag)
		{
			case MT3_TAG_BYTE:
			case MT3_TAG_SHORT:
			case MT3_TAG_INT:
			case MT3_TAG_LONG:
			case MT3_TAG_FLOAT:
			case MT3_TAG_DOUBLE:
			case MT3_TAG_STRING:
			{
				if(tree->tag == MT3_TAG_STRING)
				{
					_mt3_print_indent(level, "length: %lld", tree->length);
					_mt3_write_bytes(buffer, (const SPubyte*) &tree->length, sizeof(SPsize), level, SP_TRUE);
				}
				
				SP_ASSERT(tree->payload.data, "Node %lld has invalid data to write", tree->weight);
				_mt3_print_indent(level, "data: (%lld byte(s))", tree->length);
				_mt3_write_bytes(buffer, (const SPubyte*)tree->payload.data, tree->length, level, tree->tag != MT3_TAG_STRING);
				break;
			}
			
			case MT3_TAG_BYTE_ARRAY:
			case MT3_TAG_SHORT_ARRAY:
			case MT3_TAG_INT_ARRAY:
			case MT3_TAG_LONG_ARRAY:
			case MT3_TAG_FLOAT_ARRAY:
			case MT3_TAG_DOUBLE_ARRAY:
			{
				_mt3_print_indent(level, "length: %lld", tree->length);
				_mt3_write_bytes(buffer, (const SPubyte*) &tree->length, sizeof(SPsize), level, SP_TRUE);
				
				SP_ASSERT(tree->payload.data, "Node %lld has invalid data to write", tree->weight);
				_mt3_print_indent(level, "data: (%lld byte(s))", tree->length);
				SPsize stride = _mt3_length_of(tree->tag & ~MT3_TAG_ARRAY);
				for(SPsize i = 0; i < tree->length / stride; i++)
					_mt3_write_bytes(buffer, (const SPubyte*)tree->payload.data + stride * i, stride, level, SP_TRUE);
				break;
			}
			
			case MT3_TAG_STRING_ARRAY:
			{
				_mt3_print_indent(level, "length: %lld", tree->length);
				_mt3_write_bytes(buffer, (const SPubyte*) &tree->length, sizeof(SPsize), level, SP_TRUE);
				
				SP_ASSERT(tree->payload.data, "Node %lld has invalid data to write", tree->weight);
				_mt3_print_indent(level, "data: (%lld byte(s))", tree->length);
				SPsize i = 0;
				while(i < tree->length - 1)
				{
					SPlong length = 0;
					memcpy(&length, tree->payload.data + i, sizeof(SPlong));
					_mt3_write_bytes(buffer, (const SPubyte*) &length, sizeof(SPlong), level, SP_TRUE);
					i += sizeof(SPlong);
					_mt3_write_bytes(buffer, (const SPubyte*) tree->payload.data + i, length, level, SP_FALSE);
					i += length;
				}
				SPubyte zero = 0;
				_mt3_write_bytes(buffer, (const SPubyte*) &zero, sizeof(SPbyte), level, SP_FALSE);			
				break;
			}
			
			case MT3_TAG_ROOT:
			{
				_mt3_print_indent(level + 1, "root");
				_mt3_write_binary(tree->payload.head, buffer, level + 1);
				break;
			}
			
			
			case MT3_TAG_ROOT_ARRAY:
			{
				_mt3_print_indent(level, "length: %lld", tree->length);
				_mt3_write_bytes(buffer, (const SPubyte*) &tree->length, sizeof(SPsize), level, SP_TRUE);
				_mt3_encode_array(tree->payload.head, buffer, level + 1);
			}
			
			/*
			case MT3_TAG_ARRAY:
			{
				_mt3_print_indent(level, "length: %lld", tree->length);
				_mt3_write_bytes(buffer, (const SPubyte*) &tree->length, sizeof(SPsize), level, SP_TRUE);
				
				MT3_tree cursor = NULL;
				MT3_FOR_EACH(tree->payload.head, cursor)
				{
					_mt3_encode(cursor, buffer, level + 1);
				}
				break;
			}
			*/
		}
	}
}
static void _mt3_write_binary(const MT3_tree tree, SPbuffer* buffer, int level)
{
	if(!tree)
	{
		//signal that this node is done..
		_mt3_print_indent(level, "(end)");

		//encode the tag
		MT3_tag null = MT3_TAG_NULL;
		_mt3_write_bytes(buffer, (const SPubyte*) &null, sizeof(SPbyte), level, SP_FALSE);
		return;
	}

	//encode the tag..
	SPuint8 tag = tree->tag | ((tree->red & 1) << 6);
	_mt3_print_indent(level, "tag: (%s)", _mt3_tag_to_str(tree->tag));
	_mt3_write_bytes(buffer, (const SPubyte*) &tag, sizeof(SPbyte), level, SP_FALSE);

	//encode the weight..
	_mt3_print_indent(level, "weight: %lld", tree->weight);
	_mt3_write_bytes(buffer, (const SPubyte*) &tree->weight, sizeof(SPlong), level, SP_TRUE);

	
	_mt3_encode(tree, buffer, level);
	_mt3_print_indent(level + 1, "major");
	_mt3_write_binary(tree->major, buffer, level + 1);

	_mt3_print_indent(level + 1, "minor");
	_mt3_write_binary(tree->minor, buffer, level + 1);
}

/*
 *------------------------------------------------------------------------------------------------------------------
 *   writing rule
 *------------------------------------------------------------------------------------------------------------------
 *  for all nodes:
 *  1. write current tag..
 *  2. write weight..
 *  3. write data (explained below)..
 *  4. for major and minor repeat step 1 if not NULL, otherwise write MT3_TAG_NULL as tag respectively..
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
SPbuffer mt3_WriteBinary(const MT3_tree tree)
{
	SPbuffer buffer = SP_BUFFER_INIT;
	if(!tree)
	{
		errno = MT3_STATUS_BAD_VALUE;
		return buffer;
	}
	_mt3_write_binary(tree, &buffer, 0);
	SPbuffer compressed = _mt3_compress(buffer.data, buffer.length);
	spBufferFree(&buffer);
	return compressed;
}

static MT3_array _mt3_decode_array(const SPubyte** memory, SPsize* length)
{
	MT3_tag tag = MT3_TAG_NULL;
	MT3_READ_GENERIC(&tag, sizeof(SPubyte), _mt3_memcpy, return NULL);
	MT3_array array = NULL;
	while(tag != MT3_TAG_NULL)
	{
		switch(tag)
		{
			case MT3_TAG_ROOT:
			{
				
			}
		}
		MT3_READ_GENERIC(&tag, sizeof(SPubyte), _mt3_memcpy, return NULL);
	}
}
static SPbool _mt3_decode(MT3_tree tree, const SPubyte** memory, SPsize* length)
{
	if(tree)
	{
		switch(tree->tag)
		{
			case MT3_TAG_BYTE:
			case MT3_TAG_SHORT:
			case MT3_TAG_INT:
			case MT3_TAG_LONG:
			case MT3_TAG_FLOAT:
			case MT3_TAG_DOUBLE:
			{
				tree->length = _mt3_length_of(tree->tag);
				if(tree->length)
				{
					MT3_CHECKED_CALLOC(tree->payload.data, tree->length, sizeof(SPbyte), return SP_FALSE);
					MT3_READ_GENERIC(tree->payload.data, tree->length, _mt3_swapped_memcpy, return SP_FALSE);
				}
				break;
			}
			
			case MT3_TAG_BYTE_ARRAY:
			case MT3_TAG_STRING:
			{
				MT3_READ_GENERIC(&tree->length, sizeof(SPlong), _mt3_swapped_memcpy, return SP_FALSE);
				if(tree->length)
				{
					MT3_CHECKED_CALLOC(tree->payload.data, tree->length, sizeof(SPbyte), return SP_FALSE);
					MT3_READ_GENERIC(tree->payload.data, tree->length, _mt3_memcpy, return SP_FALSE);
				}
				break;
			}
			
			case MT3_TAG_SHORT_ARRAY:
			case MT3_TAG_INT_ARRAY:
			case MT3_TAG_LONG_ARRAY:
			case MT3_TAG_FLOAT_ARRAY:
			case MT3_TAG_DOUBLE_ARRAY:
			{
				MT3_READ_GENERIC(&tree->length, sizeof(SPlong), _mt3_swapped_memcpy, return SP_FALSE);
				if(tree->length)
				{
					MT3_CHECKED_CALLOC(tree->payload.data, tree->length, sizeof(SPbyte), return SP_FALSE);
					SPsize stride = _mt3_length_of(tree->tag & ~MT3_TAG_ARRAY);
					
					for(SPsize i = 0; i < tree->length / stride; i++)
						MT3_READ_GENERIC(tree->payload.data + i * stride, stride, _mt3_swapped_memcpy, return SP_FALSE);
				}
				break;
			}
			
			case MT3_TAG_STRING_ARRAY:
			{
				MT3_READ_GENERIC(&tree->length, sizeof(SPlong), _mt3_swapped_memcpy, return SP_FALSE);
				if(tree->length)
				{
					MT3_CHECKED_CALLOC(tree->payload.data, tree->length, sizeof(SPbyte), return SP_FALSE);
					SPsize i = 0;
					while(i < tree->length - 1)
					{
						SPlong size = 0;
						MT3_READ_GENERIC(&size, sizeof(SPlong), _mt3_swapped_memcpy, return SP_FALSE);
						memcpy(tree->payload.data + i, (const SPchar*) &size, sizeof(SPlong));
						i += sizeof(SPlong);

						MT3_READ_GENERIC(tree->payload.data + i, size, _mt3_memcpy, return SP_FALSE);
						i += size;
					}

					SPbyte zero;
					MT3_READ_GENERIC(&zero, sizeof(SPbyte), _mt3_memcpy, return SP_FALSE);
					tree->payload.data[tree->length - 1] = zero;
				}
				break;
			}
			
			case MT3_TAG_ROOT:
			{
				SP_DEBUG("root read");
				tree->payload.head = _mt3_read_binary(memory, length);
				break;
			}
			
			case MT3_TAG_ROOT_ARRAY:
			{
				MT3_READ_GENERIC(&tree->length, sizeof(SPlong), _mt3_swapped_memcpy, return SP_FALSE);
				tree->payload.head = _mt3_decode_array(memory, length);
				break;
			}
			
		}
	}
	return SP_TRUE;
}

static MT3_tree _mt3_read_binary(const SPubyte** memory, SPsize* length)
{
	SPubyte tag;
	MT3_READ_GENERIC(&tag, sizeof(SPubyte), _mt3_memcpy, return NULL);

	SPbool redness = (tag >> 6) & 1;
	tag &= ~0x40;

	if(tag == MT3_TAG_NULL)
		return NULL;

	MT3_tree tree;
	MT3_CHECKED_CALLOC(tree, 1, sizeof(MT3_node), return NULL);
	MT3_READ_GENERIC(&tree->weight, sizeof(SPlong), _mt3_swapped_memcpy, return NULL);
	tree->red = redness;
	tree->tag = tag;

	if(!_mt3_decode(tree, memory, length))
		return NULL;
	
	tree->major = _mt3_read_binary(memory, length);
	tree->minor = _mt3_read_binary(memory, length);

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

MT3_tree mt3_ReadBinary(SPbuffer buffer)
{
	SPbuffer decompressed = _mt3_decompress(buffer.data, buffer.length);
	const SPubyte** memory = (const SPubyte**) &decompressed.data;
	SPsize length = decompressed.length;

	return _mt3_read_binary(memory, &length);
}

/*<==========================================================>*
 *  error signaling
 *<==========================================================>*/
MT3_status mt3_GetLastError()
{
    MT3_status status = errno;
	errno = MT3_STATUS_OK;
	return status;
}

const char* mt3_GetErrorInfo(MT3_status status)
{
    switch(status)
    {
        case MT3_STATUS_NO_MEMORY: return "Your local machine has insufficient memory available";
        case MT3_STATUS_WRITE_ERROR: return "Error while reading data";
        case MT3_STATUS_READ_ERROR: return "Error while writing data";
        case MT3_STATUS_BAD_NAME: return "Given name was either empty, NULL, or already assigned to a node";
        case MT3_STATUS_BAD_VALUE: return "Given value was invalid";
        case MT3_STATUS_BAD_TAG: return "Given tag was invalid";
    }
    return "No errors";
}

/*<==========================================================>*
 *  tree operations
 *<==========================================================>*/

static MT3_node* _mt3_find_max(MT3_node* n)
{
	return n ? (!n->major ? n : _mt3_find_max(n->major)) : NULL;
}

static MT3_node* _mt3_find_min(MT3_node* n)
{
	return n ? (!n->minor ? n : _mt3_find_min(n->minor)) : NULL;
}

static SPbool _mt3_is_major(const MT3_node* node)
{
	return (node && node->parent) ? (node->parent->major == node) : SP_FALSE;
}

static SPbool _mt3_is_root(const MT3_node* node)
{
	return (node && !node->parent);
}

static MT3_node* _mt3_find_member(MT3_node* node)
{
	SP_ASSERT(!_mt3_is_root(node->parent), "Oopsie, this should not have happened");
	return _mt3_is_major(node->parent) ? node->parent->parent->minor : node->parent->parent->major;
}

static SPsize _mt3_calculate_black_depth(const MT3_node* rbt)
{
	if(!rbt)
		return 1;

	SPsize count = 0;
	if(!rbt->red)
		count++;
	SPsize majorDepth = _mt3_calculate_black_depth(rbt->major);
	SPsize minorDepth = _mt3_calculate_black_depth(rbt->minor);
	return count + MT3_MAX(minorDepth, majorDepth);
}

static SPbool _mt3_verify_rbt_impl(const MT3_node* rbt, SPsize depth, SPsize ref)
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
		{
			++depth;
		}

		if(rbt->major || rbt->minor)
		{
			return _mt3_verify_rbt_impl(rbt->major, depth, ref) && _mt3_verify_rbt_impl(rbt->minor, depth, ref);
		}
		else
		{
			return !rbt->red ? (ref == depth) : SP_TRUE;
		}
	}
	return SP_TRUE;
}

SPbool mt3_VerifyRBT(MT3_node* rbt)
{
	SPsize depth = _mt3_calculate_black_depth(rbt) - 1;
	return _mt3_verify_rbt_impl(rbt, 0, depth);
}

static void _mt3_fix_rbt_violations(MT3_node* node, MT3_tree* head)
{
	if(node)
	{
		if(node->red && node->parent->red)
		{
			MT3_node* m = _mt3_find_member(node);
			MT3_node* p = node->parent;
			MT3_node* g = p->parent;
			MT3_node* r = NULL;

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
				 SPbool pMajor = _mt3_is_major(p);
				 SPbool nMajor = _mt3_is_major(node);

				 if(!pMajor && !nMajor)
				 {
					// R-rotation..
					 r = _mt3_rotate_right(node->parent->parent, head);
				 }
				 else if(!pMajor && nMajor)
				 {
					// LR-rotation..
					r = _mt3_rotate_left(node->parent, head);
					    _mt3_rotate_right(node->parent, head);
				 }
				 else if(pMajor && !nMajor)
				 {
					// RL-rotation..
					r = _mt3_rotate_right(node->parent, head);
					    _mt3_rotate_left(node->parent, head);
				 }
				 else
				 {
					// L-rotation..
					r = _mt3_rotate_left(node->parent->parent, head);
				 }
				 r->red = !r->red;
				 pMajor ? (r->minor->red = !r->minor->red) : (r->major->red = !r->major->red);
			}
			_mt3_fix_rbt_violations(r, head);
		}
	}
}

static MT3_node* _mt3_rotate_left(MT3_node* n, MT3_tree* head)
{
	SP_ASSERT(n, "Expected rotation node");
	SPbool maj = _mt3_is_major(n);
	MT3_node* p = n->parent;
	MT3_node* m = n->major;
	MT3_node* c = m ? n->major->minor : NULL;

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

static MT3_node* _mt3_rotate_right(MT3_node* n, MT3_tree* head)
{
	SP_ASSERT(n, "Expected rotation node");
	SPbool maj = _mt3_is_major(n);
	MT3_node* p = n->parent;
	MT3_node* m = n->minor;
	MT3_node* c = m ? n->minor->major : NULL;

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

static void _mt3_bst_delete_impl(MT3_node* n, MT3_tree* head, MT3_node** _r, MT3_node** _x, MT3_node** _w)
{
	if(n)
	{
		MT3_node* x = NULL;
		MT3_node* w = NULL;
		MT3_node* r = NULL;
		MT3_node* p = n->parent;

		SPbool maj = _mt3_is_major(n);
		SPbool root = _mt3_is_root(n);

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
#ifndef MT3_HAVE_BST_MAJOR_INCLINED
			r = _mt3_find_min(n->major);
			w = r->parent ? (_mt3_is_major(r) ? r->parent->minor : r->parent->major) : NULL;

			SP_ASSERT(r, "Expected to have replacement");
			SP_ASSERT(!r->minor, "Expected to have minimum replacement");
			MT3_node *q = n->minor;
			MT3_node *m = n->major;
			MT3_node *a = r->parent;
			MT3_node *b = r->major;

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
			r = _mt3_find_max(n->minor);
			w = r->parent ? (_mt3_is_major(r) ? r->parent->minor : r->parent->major) : NULL;
			SP_ASSERT(r, "Expected to have replacement");
			SP_ASSERT(!r->major, "Expected to maximum replacement");

			MT3_node *q = n->major;
			MT3_node *m = n->minor;
			MT3_node *a = r->parent;
			MT3_node *b = r->minor;

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

		_mt3_delete_node(n);
	}
}

SPbool mt3_Delete(MT3_tree* tree, const SPchar* name)
{
	MT3_node* n = mt3_Search(*tree, name);
	if(n)
	{
		SPbool red = n->red;
		MT3_node* r = NULL;
		MT3_node* x = NULL;
		MT3_node* w = NULL;
		_mt3_bst_delete_impl(n, tree, &r, &x, &w);

		if(x && w)
		{
			SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
		}
		return _mt3_fix_up_rbt(red, r, x, w, tree);
	}
	return SP_FALSE;
}

static SPbool _mt3_fix_up_rbt(SPbool redBefore, MT3_node* r, MT3_node* x, MT3_node* w, MT3_tree* head)
{
	SPbool redAfter = r ? r->red : SP_FALSE;
	if(redBefore && redAfter)
		return SP_TRUE;

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

	if(_mt3_is_root(x))
		return SP_TRUE;

	return _mt3_transplant_rbt(x, w, head);
}

static SPbool _mt3_transplant_rbt(MT3_node* x, MT3_node* w, MT3_tree* head)
{
	SPbool xRed = x ? x->red : SP_FALSE;
	if(xRed)
	{
		return _mt3_transplant_proc_0(x);
	}
	else
	{
		if(!w)
		{
			// following cases would expect w to have children..
	        	// since double black cannot have children, return here..
			return SP_TRUE;
		}

		SPbool maj = !_mt3_is_major(w);
		if(w->red)
		{
			// x is black and w is red..
			return _mt3_transplant_proc_1(x, w, head);
		}
		else
		{
			SPbool wMjB = w->major ? !w->major->red : SP_TRUE;
			SPbool wMnB = w->minor ? !w->minor->red : SP_TRUE;

			if(wMjB && wMnB)
			{
				return _mt3_transplant_proc_2(x, w, head);
			}

			SPbool c = maj ? (!wMjB && wMnB) : (!wMnB && wMjB);
			if(c)
			{
				return _mt3_transplant_proc_3(x, w, head);
			}

			c = maj ? !wMnB : !wMjB;
			if(c)
			{
				return _mt3_transplant_proc_4(x, w, head);
			}
		}
	}

	return SP_FALSE;
}

SPbool _mt3_transplant_proc_0(MT3_node* x)
{
	if(x->red)
	{
		x->red = SP_FALSE;
		return SP_TRUE;
	}

	return SP_FALSE;
}

SPbool _mt3_transplant_proc_1(MT3_node* x, MT3_node* w, MT3_tree* head)
{
	if(w)
	{
		SPbool xBlack = x ? !x->red : SP_TRUE;
		SPbool maj = !_mt3_is_major(w);
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
			MT3_node* m = maj ? w->major : w->minor;
			MT3_node* p = maj ? _mt3_rotate_right(w->parent, head) : _mt3_rotate_left(w->parent, head);
			SP_ASSERT(p, "Expeceted to have rotation replacement");
            		SP_ASSERT(p == w, "Rotation error");
			p = maj ? p->major : p->minor;
			SP_ASSERT(p, "Expected to have parent");
			x = maj ? p->major : p->minor;
			w = maj ? p->minor : p->major;
			SP_ASSERT(m == w, "Rotation error");
			return _mt3_transplant_rbt(x, w, head);
		}
	}
	return SP_FALSE;
}

SPbool _mt3_transplant_proc_2(MT3_node* x, MT3_node* w, MT3_tree* head)
{
	if(w)
	{
		// x is black and w is black..
		// x could be double black, w cannot be double black..
		SPbool xBlack = x ? !x->red : SP_TRUE;
		if(xBlack && !w->red)
		{
			// only x could be double black..
			SPbool maj = !_mt3_is_major(w);
			SPbool wMnB = w->minor ? !w->minor->red : SP_TRUE;
			SPbool wMjB = w->major ? !w->major->red : SP_TRUE;
			if(wMnB && wMjB)
			{
				w->red = SP_TRUE;
				x = w->parent;
				if(_mt3_is_root(x))
				{
					x->red = SP_FALSE;
					return SP_TRUE;
				}
				maj = _mt3_is_major(x);
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
					return _mt3_transplant_rbt(x, w, head);
				}
			}
		}
	}
	return SP_FALSE;
}

SPbool _mt3_transplant_proc_3(MT3_node* x, MT3_node* w, MT3_tree* head)
{
	if(w)
	{
		SPbool xBlack = x ? !x->red : SP_TRUE;
		SPbool wMjB = w->major ? !w->major->red : SP_TRUE;
		SPbool wMnB = w->minor ? !w->minor->red : SP_TRUE;
		SPbool maj = !_mt3_is_major(w);
		SPbool c = maj ? (!wMjB && wMnB) : (wMjB && !wMnB);

		if(xBlack && c)
		{
			if(x)
			{
				SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
			}
			maj = !_mt3_is_major(w);
			SP_ASSERT(maj ? w->major : w->minor, "Expected sibling's child");
			maj ? (w->major->red = SP_FALSE) : (w->minor->red = SP_FALSE);
			w->red = SP_TRUE;
			w = maj ? _mt3_rotate_left(w, head) : _mt3_rotate_right(w, head);
			SP_ASSERT(w, "Expected sibling");
            		SP_ASSERT(w->parent, "Expected parent");
			x = maj ? w->parent->major : w->parent->minor;
			SP_ASSERT(w != x, "Sibling and replacement cannot be the same");
			return _mt3_transplant_proc_4(x, w, head);
		}
	}
	return SP_FALSE;
}

SPbool _mt3_transplant_proc_4(MT3_node* x, MT3_node* w, MT3_tree* head)
{
	if(w)
	{
		// x can be double black..
		// w cannot be double black..
		SPbool wMjR = w->major ? w->major->red : SP_FALSE;
		SPbool wMnR = w->minor ? w->minor->red : SP_FALSE;
		SPbool xBlack = x ? !x->red : SP_TRUE;
		SPbool maj = !_mt3_is_major(w);
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
			maj ? _mt3_rotate_right(w->parent, head) : _mt3_rotate_left(w->parent, head);
			return SP_TRUE;
		}
	}
	return SP_FALSE;
}

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic pop
#endif
