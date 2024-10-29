#include "SP/sparse/mot.h"
#include <errno.h>

/*
 *  root nodes should have tag MOT_TAG_ROOT (there is always only one root-node for each tree)!!
 *  branch nodes have a payload of NULL but connect the tree further..
 *  to serialize/deserialize read or write all major branches first and then all minor branches..
 *  this makes the tree one-dimensional..
 *  append the NULL tag to the end of each branch..
 */

/*
 *------------------------------------------------------------------------------------------------------------------
 *   writing rule
 *------------------------------------------------------------------------------------------------------------------
 *  for all nodes:
 *  write current tag..
 *  write weight..
 *  write data (explained below)..
 *  for major and minor repeat step 1 if not NULL, otherwise write for each 0 as tag..
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
		
#define MOT_READ_GENERIC(dst, n, scanner, on_failure)            \
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

MOT_tree* motAllocTree(const SPchar* name)
{
	struct MOT_node* root = NULL;
	MOT_CHECKED_CALLOC(root, 1, sizeof(struct MOT_node), return NULL);

	if(!name)
	{
		SP_WARNING("Tree must have a name");
		return NULL;
	}
	root->weight = _mot_sdbm(name);
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

static void _mot_insert_bytes(MOT_tree* tree, SPlong weight, MOT_tag tag, SPsize length, const SPbyte* value)
{
	if(weight == tree->weight)
	{
		SP_DEBUG("Name has already been given to a node");
		return;
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
}

static void _mot_insert_root(MOT_tree* tree, MOT_tree* value)
{
    SP_ASSERT(tree, "Cannot Insert integer to empty tree");
    SP_ASSERT(value->weight != tree->weight, "Name has already been given to a node");
    SP_ASSERT(tree->weight != MOT_TAG_ROOT, "Cannot insert data to root-or branch-nodes");

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
		_mot_insert_bytes(tree, hash, MOT_TAG_STRING, strlen(value), (const SPbyte*)value);
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
	
	if(data && length > 0)
	{
		MOT_CHECK_INPUT(name);
		_mot_insert_bytes(tree, hash, tag | MOT_TAG_ARRAY, length * _mot_length_of(tag), (const SPbyte*)data);
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

const char* _mot_type_to_str(MOT_tag tag)
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
			
			printf("%s (length: %lld stride: %lld) (%lld):\n", _mot_type_to_str(tree->tag), tree->length, stride, tree->weight);
			
			for(int i = 0; i < level + 1; i++)
				printf("\t");
			
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
					printf("Tree (%lld):\n", tree->weight);
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
					
					printf("%s (%lld): %lld\n", _mot_type_to_str(tree->tag), tree->weight, value);
					
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
					printf("%s (%lld): %f\n", _mot_type_to_str(tree->tag), tree->weight, (tree->tag == MOT_TAG_FLOAT) ? value._f : value._d);
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

static void _mot_write_bytes_native_endian(SPbuffer* buffer, const SPbyte* src, SPsize stride, SPsize amount)
{	
	SPbyte* chunk = motAllocChunk(stride * amount);
	_mot_swapped_memcpy(chunk, src, stride * amount);
	spBufferAppend(buffer, chunk, stride * amount);
	free(chunk);
}

static void _motWriteBinary(MOT_tree* tree, SPbuffer* buffer)
{
	if(!tree)
	{
		//signal that this node is done..
		MOT_tag null = MOT_TAG_NULL;
		_mot_write_bytes_native_endian(buffer, (SPbyte*) &null, sizeof(SPbyte), 1);
		return;
	}
	
	//encode the tag..
	_mot_write_bytes_native_endian(buffer, (SPbyte*) &tree->tag, sizeof(SPbyte), 1);
	
	//encode the weight..
	_mot_write_bytes_native_endian(buffer, (SPbyte*) &tree->weight, sizeof(SPlong), 1);
	
	//encode the data..
	switch(tree->tag)
	{
		case MOT_TAG_BYTE: _mot_write_bytes_native_endian(buffer, tree->payload.data, sizeof(SPbyte), tree->length); break;
		case MOT_TAG_SHORT: _mot_write_bytes_native_endian(buffer, tree->payload.data, sizeof(SPshort), tree->length); break;
		case MOT_TAG_INT: _mot_write_bytes_native_endian(buffer, tree->payload.data, sizeof(SPint), tree->length); break;
		case MOT_TAG_LONG: _mot_write_bytes_native_endian(buffer, tree->payload.data, sizeof(SPlong), tree->length); break;
		case MOT_TAG_FLOAT: _mot_write_bytes_native_endian(buffer, tree->payload.data, sizeof(SPfloat), tree->length); break;
		case MOT_TAG_DOUBLE: _mot_write_bytes_native_endian(buffer, tree->payload.data, sizeof(SPdouble), tree->length); break;
	}
	
	_motWriteBinary(tree->major, buffer);
	_motWriteBinary(tree->minor, buffer);
}
SPbuffer motWriteBinary(MOT_tree* tree)
{
	SPbuffer buffer = SP_BUFFER_INIT;
	if(!tree)
	{
		SP_WARNING("Cannot serialize empty tree");
		return buffer;
	}
	_motWriteBinary(tree, &buffer);
	return buffer;
}


static MOT_tree* _motReadBinary(const SPubyte** memory, SPsize* length)
{
	SPbyte tag;
	MOT_READ_GENERIC(&tag, sizeof(SPbyte), _mot_memcpy, return NULL);
	
	if(tag == MOT_TAG_NULL)
		return NULL;
	
	MOT_tree* tree;
	MOT_CHECKED_CALLOC(tree, 1, sizeof(MOT_tree), return NULL);
	MOT_READ_GENERIC(&tree->weight, sizeof(SPlong), _mot_swapped_memcpy, return NULL);
	tree->tag = tag;
	
	if(tag != MOT_TAG_ROOT)
	{
		SPsize readLength = 0;
		switch(tag)
		{
			case MOT_TAG_BYTE: readLength = sizeof(SPbyte); break;
			case MOT_TAG_SHORT: readLength = sizeof(SPshort); break;
			case MOT_TAG_INT: readLength = sizeof(SPint); break;
			case MOT_TAG_LONG: readLength = sizeof(SPlong); break;
		}
		if(readLength)
		{
			MOT_CHECKED_CALLOC(tree->payload.data, readLength, sizeof(SPbyte), return NULL);
			MOT_READ_GENERIC(tree->payload.data, readLength, _mot_swapped_memcpy, return NULL);
			tree->length = readLength;
		}
	}
	tree->major = _motReadBinary(memory, length);
	tree->minor = _motReadBinary(memory, length);
	return tree;
}

MOT_tree* motReadBinary(SPbuffer buffer)
{
	const SPubyte** memory = (const SPubyte**) &buffer.data;
	SPsize* length = &buffer.length;
	
	return _motReadBinary(memory, length);
}

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic pop
#endif