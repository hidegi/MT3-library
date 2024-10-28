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
 *  write id..
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
 *  read id..
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
#define MOT_SCALAR_MASK 0x1FF
#define MOT_CHECKED_CALLOC(ptr, n, size, on_error)	\
	do												\
	{												\
		if(!((ptr) = calloc(n, size)))				\
		{											\
			errno = MOT_ERR_MEM;					\
			on_error;								\
		}											\
	} while(0)
		
#define MOT_SET_ID(dst, src, on_error)           	\
	do												\
	{                                              	\
	    if(!src)                                	\
	    {                                      		\
	        errno = MOT_ERR_GEN;					\
	        on_error;								\
	    }                                          	\
		(dst)->id = _motHashSDBM(src);				\
	} while(0)
		
#define MOT_ALLOC_TYPE(_tag, _length, ptr, v)								\
	do																		\
	{																		\
		struct MOT_node* node = NULL;										\
		MOT_CHECKED_CALLOC(node, 1, sizeof(struct MOT_node), return);		\
		node->tag = _tag;													\
		node->id = name;													\
		node->length = (_length);							                \
		node->major = node->minor = NULL;									\
		MOT_CHECKED_CALLOC(node->data, 						                \
			node->length, sizeof(SPbyte), return);			                \
		memcpy(node->data, (v), 								            \
			node->length * sizeof(SPbyte));					                \
		(ptr) = node;														\
	} while(0)

long long _motHashSDBMImpl(const char *str) {
    long long hash = 0;
    int c;
    while ((c = *str++)) {
        hash = c + (hash << 6) + (hash << 16) - hash; // hash * 65599 + c
    }

    return hash;
}

long long _motHashSDBM(const char* str)
{
	SPulong hash = _motHashSDBMImpl(str);
    SPbyte buffer[4];
    // Split the hash into 8 bytes
    for (int i = 0; i < 8; i++) {
        buffer[i] = (hash >> (i * 8)) & 0xFF; // Get the i-th byte
    }
	
	long long output = 0;
	memcpy(&output, buffer, sizeof(SPbyte) * 4);
	return output;
}

MOT_tree* motAllocTree(const SPchar* name)
{
	struct MOT_node* root = NULL;
	MOT_CHECKED_CALLOC(root, 1, sizeof(struct MOT_node), return NULL);
	MOT_SET_ID(root, name, return NULL);
	root->tag = MOT_TAG_ROOT;
	root->major = root->minor = NULL;
	return root;
}

void motFreeTree(MOT_tree* tree)
{
	if(tree)
	{
		free(tree->data);
		motFreeTree(tree->major);
		motFreeTree(tree->minor);
		SP_DEBUG("freeing %lld", tree->id);
		free(tree);
	}
}

#define MOT_INIT_RECURSIVE(fn, v){\
	if(name > tree->id)\
		if(!tree->major)\
		{\
			MOT_ALLOC_TYPE(tag, length, tree->major, (v));\
		}\
		else\
		{\
		    if(tree->tag != MOT_TAG_BRANCH)\
			    fn(tree->major, name, tag, length, value);\
            else\
            {\
                struct MOT_node* node;\
                MOT_ALLOC_TYPE(tag, length, node, (v));\
                if(tree->major->id < name)\
                    node->minor = tree->major;\
                else\
                    node->major = tree->major;\
                node->minor = NULL;\
                tree->major = node;\
            }\
		}\
	else\
		if(!tree->minor)\
			MOT_ALLOC_TYPE(tag, length, tree->minor, (v));\
		else\
		{\
		    if(tree->tag != MOT_TAG_BRANCH)\
		        fn(tree->minor, name, tag, length, value);\
            else\
            {\
                struct MOT_node* node;\
                MOT_ALLOC_TYPE(tag, length, node, (v));\
                if(tree->minor->id < name)\
                    node->minor = tree->minor;\
                else\
                    node->major = tree->minor;\
                node->major = NULL;\
                tree->minor = node;\
            }\
        }\
	}


MOT_tree* _motSearch(MOT_tree* tree, SPlong hash)
{
	MOT_tree* node = NULL;
	if(tree)
	{
		if(tree->id == hash)
			return tree;
		if((node = _motSearch(tree->major, hash)))
			return node;
		
		if((node = _motSearch(tree->minor, hash)))
			return node;
	}
	return NULL;
}

MOT_tree* motSearch(MOT_tree* tree, const char* name)
{
	long long hash = _motHashSDBM(name);
	return _motSearch(tree, hash);
}
void _motInsertInteger(MOT_tree* tree, SPlong name, MOT_tag tag, SPsize length, SPlong value)
{
    MOT_INIT_RECURSIVE(_motInsertInteger, &value);
}

void _motInsertFloat(MOT_tree* tree, SPlong name, MOT_tag tag, SPsize length, SPfloat value)
{
    MOT_INIT_RECURSIVE(_motInsertFloat, &value);
}

void _motInsertDouble(MOT_tree* tree, SPlong name, MOT_tag tag, SPsize length, SPdouble value)
{
    MOT_INIT_RECURSIVE(_motInsertDouble, &value);
}

void motInsertByte(MOT_tree* tree, const SPchar* name, SPbyte value)
{
	long long hash = _motHashSDBM(name);
	_motInsertInteger(tree, hash, MOT_TAG_BYTE, sizeof(SPbyte), (SPlong) value);
}

void motInsertShort(MOT_tree* tree, const SPchar* name, SPshort value)
{
	long long hash = _motHashSDBM(name);
	_motInsertInteger(tree, hash, MOT_TAG_SHORT, sizeof(SPshort), (SPlong) value);
}

void motInsertInt(MOT_tree* tree, const SPchar* name, SPint value)
{
	long long hash = _motHashSDBM(name);
	_motInsertInteger(tree, hash, MOT_TAG_INT, sizeof(SPint), (SPlong) value);
}

void motInsertLong(MOT_tree* tree, const SPchar* name, SPlong value)
{
	long long hash = _motHashSDBM(name);
	_motInsertInteger(tree, hash, MOT_TAG_LONG, sizeof(SPlong), value);
}

void motInsertFloat(MOT_tree* tree, const SPchar* name, SPfloat value)
{
	long long hash = _motHashSDBM(name);
	_motInsertFloat(tree, hash, MOT_TAG_FLOAT, sizeof(SPfloat), value);
}

void motInsertDouble(MOT_tree* tree, const SPchar* name, SPdouble value)
{
	long long hash = _motHashSDBM(name);
	_motInsertDouble(tree, hash, MOT_TAG_DOUBLE, sizeof(SPdouble), value);
}


void _motInsertBytes(MOT_tree* tree, SPlong name, MOT_tag tag, SPsize length, const SPchar* value)
{
    MOT_INIT_RECURSIVE(_motInsertBytes, value);
}

void motInsertString(MOT_tree* tree, const SPchar* name, const SPchar* value)
{
	if(value) 
	{
		long long hash = _motHashSDBM(name);
		_motInsertBytes(tree, hash, MOT_TAG_STRING, strlen(value), value);
	}
}

void motInsertByteArray(MOT_tree* tree, const SPchar* name, MOT_byte_array value)
{
	if(value.data && value.length > 0)
	{
		long long hash = _motHashSDBM(name);
		_motInsertBytes(tree, hash, MOT_TAG_BYTE | MOT_TAG_ARRAY, value.length * sizeof(SPbyte), value.data);
	}
}

void motInsertIntArray(MOT_tree* tree, const SPchar* name, MOT_int_array value)
{
	if(value.data && value.length > 0)
	{
		long long hash = _motHashSDBM(name);
		_motInsertBytes(tree, hash, MOT_TAG_INT | MOT_TAG_ARRAY, value.length * sizeof(SPint), (SPbyte*)value.data);
	}
}

void motInsertLongArray(MOT_tree* tree, const SPchar* name, MOT_long_array value)
{
	if(value.data && value.length > 0)
	{
		long long hash = _motHashSDBM(name);
		_motInsertBytes(tree, hash, MOT_TAG_LONG | MOT_TAG_ARRAY, value.length * sizeof(SPlong), (SPbyte*)value.data);
	}
}
/*<==========================================================>*
 *  debug
 *<==========================================================>*/
MOT_byte_array motAllocByteArray(SPsize length)
{
	MOT_byte_array array;
	array.length = length;
	MOT_CHECKED_CALLOC(array.data, length, sizeof(SPbyte), ;);
	return array;
}

MOT_int_array motAllocIntArray(SPsize length)
{
	MOT_int_array array;
	array.length = length;
	MOT_CHECKED_CALLOC(array.data, length, sizeof(SPint), ;);
	return array;
}

MOT_long_array motAllocLongArray(SPsize length)
{
	MOT_long_array array;
	array.length = length;
	MOT_CHECKED_CALLOC(array.data, length, sizeof(SPlong), ;);
	return array;
}


SP_API MOT_int_array motAllocIntArray(SPsize length);
SP_API MOT_long_array motAllocLongArray(SPsize length);
const char* _motGetTypeStr(MOT_tag tag)
{
	if(tag & MOT_TAG_ARRAY)
	{
		MOT_tag scalar = tag & 0x1ff;
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
        case MOT_TAG_BRANCH: return "branch";
    }
	

    return "null";
}

SPsize _motBaseLength(MOT_tag tag)
{
    switch(tag)
    {
        case MOT_TAG_BYTE: return sizeof(SPbyte);
        case MOT_TAG_SHORT: return sizeof(SPshort);
        case MOT_TAG_INT: return sizeof(SPint);
        case MOT_TAG_LONG: return sizeof(SPlong);
        case MOT_TAG_FLOAT: return sizeof(SPfloat);
        case MOT_TAG_DOUBLE: return sizeof(SPdouble);
    }
    return 0;
}

void motPrintTreeImpl(const MOT_tree* tree, int level)
{
	if(tree)
	{
		for(int i = 0; i < level; i++)
            printf("\t");
		
		if(tree->tag & MOT_TAG_ARRAY)
		{
			MOT_tag scalar = tree->tag & ~MOT_TAG_ARRAY;
			SPsize stride = _motBaseLength(tree->tag & ~MOT_TAG_ARRAY);
			
			printf("%s (length: %lld stride: %lld) (%lld):\n", _motGetTypeStr(tree->tag), tree->length, stride, tree->id);
			for(int i = 0; i < level + 1; i++)
				printf("\t");
			
			for(SPsize i = 0; i < tree->length; i += stride)
			{
				SPlong l = 0;
				memcpy(&l, tree->data + i, stride * sizeof(SPbyte));
				printf("%lld ", l);
			}
			printf("\n");
		}
		else
		{
			switch(tree->tag)
			{
				case MOT_TAG_ROOT:
				case MOT_TAG_BRANCH:
				{
					printf("Tree (%lld):\n", tree->id);
					break;
				}
				case MOT_TAG_BYTE:
				case MOT_TAG_SHORT:
				case MOT_TAG_INT:
				case MOT_TAG_LONG:
				{
					SPbyte* buffer = tree->data;
					SP_ASSERT(buffer, "Node has invalid data");
					SP_ASSERT(tree->length <= 8 && tree->length > 0, "Invalid data size for an integer");
					
					long value = 0;
					memcpy(&value, buffer, tree->length * sizeof(SPbyte));
					
					printf("%s (%lld): %lld\n", _motGetTypeStr(tree->tag), tree->id, value);
					
					break;
				}
				
				case MOT_TAG_FLOAT:
				case MOT_TAG_DOUBLE:
				{
					SPbyte* buffer = tree->data;
					SP_ASSERT(buffer, "Node has invalid data");
					union
					{
						float _f;
						double _d;
					} value;
					memcpy(((tree->tag == MOT_TAG_FLOAT) ? (void*)&value._f : (void*)&value._d), buffer, tree->length * sizeof(SPbyte));
					printf("%s (%lld): %f\n", _motGetTypeStr(tree->tag), tree->id, (tree->tag == MOT_TAG_FLOAT) ? value._f : value._d);
					break;
				}
				
				case MOT_TAG_STRING:
				{
					SPbyte* buffer = tree->data;
					SP_ASSERT(buffer, "Node has invalid data");
					printf("string (%lld): %s\n", tree->id, buffer);
					break;
				}
			}
		}
		motPrintTreeImpl(tree->major, level + 1);
		motPrintTreeImpl(tree->minor, level + 1);
	}
}
void motPrintTree(const MOT_tree* tree)
{
	motPrintTreeImpl(tree, 0);
	printf("\n");
}