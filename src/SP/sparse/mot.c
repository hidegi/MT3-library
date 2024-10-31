#include "SP/sparse/mot.h"
#include <errno.h>
#include <stdarg.h>

/*
 *  root nodes have tag MOT_TAG_ROOT..
 *  to serialize/deserialize read or write all major branches first and then all minor branches..
 *  append the NULL tag to the end of each branch..
 */

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
 *  read tag..
 *  read weight..
 *  read data..
 *  for major and minor, repeat step 1 (if not 0 as tag, respectively)..
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
#endif
#define MOT_CHECKED_CALLOC(ptr, n, size, on_error)	\
	do												\
	{												\
		if(!((ptr) = calloc(n, size)))				\
		{											\
			errno = MOT_ERR_MEM;					\
			on_error;								\
		}											\
	} while(0)
		
#define MOT_CHECKED_APPEND(b, ptr, len)    	\
    do                                      \
    {                                       \
    if(spBufferAppend((b), (ptr), (len)))   \
        return MOT_ERR_MEM;                 \
    } while(0)
		
#define MOT_DUMP_NUM(type, x)                       \
    do                                              \
    {                                               \
        type temp = x;                              \
        ne2be(&temp, sizeof temp);                  \
        MOT_CHECKED_APPEND(b, &temp, sizeof temp); 	\
    } while(0)
		
	
#define MOT_CHECK_INPUT(_name)									\
	long long hash = 0;											\
	do															\
	{															\
		SP_ASSERT(tree, "Cannot Insert data to empty tree");	\
		SP_ASSERT((_name), "Node must have a name");			\
		hash = _mot_sdbm((_name));								\
		SP_ASSERT(hash != 0, "Name cannot be empty");			\
	} while(0)
		
#define MOT_READ_GENERIC(dst, n, scanner, on_failure)           \
    do                                                          \
    {                                                           \
        if(*length < (n))                                       \
        {                                                       \
            on_failure;                                         \
        }                                                       \
        *memory = scanner((dst), *memory, (n));                 \
        *length -= n;                                           \
    } while(0)
		
#define MOT_COPY_TO_PAYLOAD(tagName) SP_READ_GENERIC(&node->payload.tagName, sizeof(node->payload.tagName), _spSwappedMemscan, goto sp_error)

#define ne2be _mot_big_endian_to_native_endian
#define be2ne _mot_big_endian_to_native_endian
static int _mot_is_little_endian()
{
    SPuint16 t = 0x0001;
    char c[2];
    memcpy(c, &t, sizeof t);
    return c[0];
}

static void* _mot_swap_bytes(void* s, SPsize length)
{
    for(char *b = s, *e = b + length - 1; b < e; b++, e--)
    {
        char t = *b;
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

struct MOT_node* _motAllocNode(MOT_tag tag, SPlong name, SPsize length, const SPbyte* value)
{
		struct MOT_node* node = NULL;
	    MOT_CHECKED_CALLOC(node, 1, sizeof(struct MOT_node), return NULL);
		node->tag = tag;
		node->weight = name;
		node->length = length;
		node->major = node->minor = NULL;

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

static SPlong _mot_sdbm_impl(const SPchar *str) {
    SPlong hash = 0;
    int c;
    while ((c = *str++)) {
        hash = c + (hash << 6) + (hash << 16) - hash; // hash * 65599 + c
    }

    return hash;
}

static SPlong _mot_sdbm(const SPchar* str)
{
	SPulong hash = _mot_sdbm_impl(str);
    SPbyte buffer[4];
    // Split the hash into 8 bytes
    for (SPsize i = 0; i < 8; i++) {
        buffer[i] = (hash >> (i * 8)) & 0xFF; // Get the i-th byte
    }
	
	SPlong output = 0;
	memcpy(&output, buffer, sizeof(SPbyte) * 4);
	return output;
}

void _mot_print_binary(const unsigned char *byteArray, size_t size, int level) {
    for (size_t i = 0; i < size; i++) {
        unsigned char byte = byteArray[i];
        for(int i = 0; i < level; i++)
            printf("\t");
        printf("    ");
        // Print the byte in binary
        for (int j = 7; j >= 0; j--) {
            // Use bitwise AND and right shift to get each bit
            printf("%d", (byte >> j) & 1);
        }
        printf("\n"); // New line after each byte
    }
}

MOT_tree* motAllocTree(const SPchar* name)
{
	struct MOT_node* root = NULL;
	MOT_CHECKED_CALLOC(root, 1, sizeof(struct MOT_node), return NULL);

	if(!name)
	{
		SP_WARNING("Tree must have a name");
		free(root);
		return NULL;
	}
	root->weight = _mot_sdbm(name);
	if(root->weight == 0)
	{
	    SP_WARNING("Name cannot be empty");
	    free(root);
	    return NULL;
	}
	root->tag = MOT_TAG_ROOT;
	root->major = root->minor = NULL;
	return root;
}

void motFreeTree(MOT_tree* tree)
{
	if(tree)
	{
		if(tree->tag != MOT_TAG_ROOT)
			free(tree->payload.data);
		else
		{
			motFreeTree(tree->payload.branch.major);
			motFreeTree(tree->payload.branch.minor);
		}
		motFreeTree(tree->major);
		motFreeTree(tree->minor);
		free(tree);
	}
}

static MOT_tree* _mot_search_for(MOT_tree* tree, SPlong hash)
{
	if(tree)
	{
		if(tree->weight == hash)
			return tree;
			
		MOT_tree* node = NULL;
		if((node = _mot_search_for(tree->major, hash)))
			return node;
		
		if((node = _mot_search_for(tree->minor, hash)))
			return node;
	}
	return NULL;
}

MOT_tree* motSearch(MOT_tree* tree, const char* name)
{
	long long hash = _mot_sdbm(name);
	return _mot_search_for(tree, hash);
}

static MOT_tree* _mot_insert_bytes(MOT_tree* tree, SPlong weight, MOT_tag tag, SPsize length, const SPbyte* value)
{
	if(weight == tree->weight)
	{
		SP_DEBUG("Name has already been given to a node");
		return NULL;
	}
	
    SPbool isMajor = (weight > tree->weight);
    MOT_tree* primary  = isMajor ? tree->major : tree->minor;
	
	if(primary)
		_mot_insert_bytes(primary, weight, tag, length, value);
    else
    {
        primary = _motAllocNode(tag, weight, length, value);
        isMajor ? (tree->major = primary) : (tree->minor = primary);
    }
	return primary;
}

static void _mot_insert_root(MOT_tree* tree, MOT_tree* value)
{
    SP_ASSERT(tree, "Cannot insert tree to empty tree");
    SP_ASSERT(value, "Cannot insert empty to empty tree");
    SP_ASSERT(value->weight != tree->weight, "Name has already been given to a node");
    SP_ASSERT(tree->tag == MOT_TAG_ROOT, "Root-node must be a tree");
    SP_ASSERT(value->tag == MOT_TAG_ROOT, "Inserted node must be a tree");

    SPbool isMajor = (value->weight > tree->weight);
    MOT_tree* primary  = isMajor ? tree->major : tree->minor;
	
	if(primary)
		_mot_insert_root(primary, value);
    else
    {
		primary = _motAllocNode(MOT_TAG_ROOT, value->weight, 0, NULL);
		primary->payload.branch.major = value->major;
		primary->payload.branch.minor = value->minor;
		free(value);
        isMajor ? (tree->major = primary) : (tree->minor = primary);
    }
}

void motInsertTree(MOT_tree* tree, MOT_tree* value)
{
	SP_ASSERT(value, "Cannot insert empty tree");
	SP_ASSERT(value->tag == MOT_TAG_ROOT, "Invalid tag for root-node");
	SP_ASSERT(!value->payload.data, "Root-nodes must not contain data");
	_mot_insert_root(tree, value);
}
	
void motInsertByte(MOT_tree* tree, const SPchar* name, SPbyte value)
{
	MOT_CHECK_INPUT(name);
	_mot_insert_bytes(tree, hash, MOT_TAG_BYTE, sizeof(SPbyte), (const SPbyte*) &value);
}

void motInsertShort(MOT_tree* tree, const SPchar* name, SPshort value)
{
	MOT_CHECK_INPUT(name);
	_mot_insert_bytes(tree, hash, MOT_TAG_SHORT, sizeof(SPshort), (const SPbyte*) &value);
}

void motInsertInt(MOT_tree* tree, const SPchar* name, SPint value)
{
	MOT_CHECK_INPUT(name);
	_mot_insert_bytes(tree, hash, MOT_TAG_INT, sizeof(SPint), (const SPbyte*) &value);
}

void motInsertLong(MOT_tree* tree, const SPchar* name, SPlong value)
{
	MOT_CHECK_INPUT(name);
	_mot_insert_bytes(tree, hash, MOT_TAG_LONG, sizeof(SPlong), (const SPbyte*) &value);
}

void motInsertFloat(MOT_tree* tree, const SPchar* name, SPfloat value)
{
	MOT_CHECK_INPUT(name);
	_mot_insert_bytes(tree, hash, MOT_TAG_FLOAT, sizeof(SPfloat), (const SPbyte*) &value);
}

void motInsertDouble(MOT_tree* tree, const SPchar* name, SPdouble value)
{
	MOT_CHECK_INPUT(name);
	_mot_insert_bytes(tree, hash, MOT_TAG_DOUBLE, sizeof(SPdouble), (const SPbyte*) &value);
}

void motInsertString(MOT_tree* tree, const SPchar* name, const SPchar* value)
{
	if(value) 
	{
		MOT_CHECK_INPUT(name);
		_mot_insert_bytes(tree, hash, MOT_TAG_STRING, strlen(value) + 1, (const SPbyte*)value);
	}
}

void motInsertByteArray(MOT_tree* tree, const SPchar* name, MOT_byte_array value)
{
	if(value.data && value.length > 0)
	{
		MOT_CHECK_INPUT(name);
		_mot_insert_bytes(tree, hash, MOT_TAG_BYTE | MOT_TAG_ARRAY, value.length * sizeof(SPbyte), value.data);
	}
}

void motInsertIntArray(MOT_tree* tree, const SPchar* name, MOT_int_array value)
{
	if(value.data && value.length > 0)
	{
		MOT_CHECK_INPUT(name);
		_mot_insert_bytes(tree, hash, MOT_TAG_INT | MOT_TAG_ARRAY, value.length * sizeof(SPint), (SPbyte*)value.data);
	}
}

void motInsertLongArray(MOT_tree* tree, const SPchar* name, MOT_long_array value)
{
	if(value.data && value.length > 0)
	{
		MOT_CHECK_INPUT(name);
		_mot_insert_bytes(tree, hash, MOT_TAG_LONG | MOT_TAG_ARRAY, value.length * sizeof(SPlong), (SPbyte*)value.data);
	}
}

void motInsertArray(MOT_tree* tree, const SPchar* name, MOT_tag tag, SPsize length, const void* data)
{
	if(tag == MOT_TAG_ROOT || tag == MOT_TAG_ROOT)
	{
		SP_WARNING("Tag \"MOT_TAG_TREE\" or \"MOT_TAG_ROOT\" invalid with \"MOT_TAG_ARRAY\"");
		return;
	}
	
	if(!(data && length > 0))
	{
		SP_WARNING("Cannot feed with empty array");
		return;
	}
	
	if(!(tag & ~MOT_TAG_ARRAY))
	{
		SP_WARNING("Expected type");
		return;
	}
	
	MOT_CHECK_INPUT(name);
	if(tag != MOT_TAG_STRING)
	{	
		_mot_insert_bytes(tree, hash, tag | MOT_TAG_ARRAY, length * _mot_length_of(tag), (const SPbyte*)data);
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
		_mot_insert_bytes(tree, hash, tag | MOT_TAG_ARRAY, byteCount, memory);
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
			case MOT_TAG_NULL: return "null";
		}
	}
	
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

    return "null";
}

void _mot_print_tree(const MOT_tree* tree, int level)
{
	if(tree)
	{
		for(int i = 0; i < level; i++)
            printf("\t");
		
		if(tree->tag & MOT_TAG_ARRAY)
		{
			
			MOT_tag scalar = tree->tag & ~MOT_TAG_ARRAY;
			SPsize stride = _mot_length_of(tree->tag & ~MOT_TAG_ARRAY);
			
			printf("%s (length: %lld stride: %lld) (%lld): ", _mot_tag_to_str(tree->tag), tree->length, stride, tree->weight);
			
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
					printf("%lld ", l);
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
					printf("root (%lld):\n", tree->weight);
					_mot_print_tree(tree->payload.branch.major, level + 1);
					_mot_print_tree(tree->payload.branch.minor, level + 1);
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
					
					printf("%s (%lld): %lld\n", _mot_tag_to_str(tree->tag), tree->weight, value);
					
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
					printf("%s (%lld): %f\n", _mot_tag_to_str(tree->tag), tree->weight, (tree->tag == MOT_TAG_FLOAT) ? value._f : value._d);
					break;
				}
				
				case MOT_TAG_STRING:
				{
					SPbyte* buffer = tree->payload.data;
					SP_ASSERT(buffer, "Node has invalid data");
					printf("string (%lld): %s\n", tree->weight, buffer);
					break;
				}
			}
		}
		for(int i = 0; i < level + 1; i++)
            printf("\t");
		printf("major:\n");
		_mot_print_tree(tree->major, level + 1);
		for(int i = 0; i < level + 1; i++)
            printf("\t");
		printf("minor:\n");
		_mot_print_tree(tree->minor, level + 1);
	}
}
void motPrintTree(const MOT_tree* tree)
{
	_mot_print_tree(tree, 0);
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

static void _motWriteBinary(MOT_tree* tree, SPbuffer* buffer, int level)
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
	_mot_print_indent(level, "tag: (%s)", _mot_tag_to_str(tree->tag));
	_mot_write_bytes(buffer, (const SPubyte*) &tree->tag, sizeof(SPbyte), level, SP_FALSE);

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
	_mot_print_indent(level + 1, "major");
	_motWriteBinary(tree->major, buffer, level + 1);

	_mot_print_indent(level + 1, "minor");
	_motWriteBinary(tree->minor, buffer, level + 1);
}
SPbuffer motWriteBinary(MOT_tree* tree)
{
	SPbuffer buffer = SP_BUFFER_INIT;
	if(!tree)
	{
		SP_WARNING("Cannot serialize empty tree");
		return buffer;
	}
	_motWriteBinary(tree, &buffer, 0);
	return buffer;
}


static MOT_tree* _motReadBinary(const SPubyte** memory, SPsize* length)
{
	SPubyte tag;
	MOT_READ_GENERIC(&tag, sizeof(SPubyte), _mot_memcpy, return NULL);
	
	if(tag == MOT_TAG_NULL)
		return NULL;

	MOT_tree* tree;
	MOT_CHECKED_CALLOC(tree, 1, sizeof(MOT_tree), return NULL);
	MOT_READ_GENERIC(&tree->weight, sizeof(SPlong), _mot_swapped_memcpy, return NULL);
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
					tree->payload.data[tree->length - 1] = 0;
				}
			}
        }
	}
	tree->major = _motReadBinary(memory, length);
	tree->minor = _motReadBinary(memory, length);
	return tree;
}

MOT_tree* motReadBinary(SPbuffer buffer)
{
	const SPubyte** memory = (const SPubyte**) &buffer.data;
	SPsize length = buffer.length;
	
	return _motReadBinary(memory, &length);
}

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic pop
#endif