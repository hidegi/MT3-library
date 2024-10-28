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
#define MOT_ARRAY_MASK 0x1FF
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

#define MOT_INIT_RECURSIVE(fn, v)                                                               \
    do                                                                                          \
    {                                                                                           \
        SP_ASSERT(tree, "Cannot Insert integer to empty tree");                                 \
        SP_ASSERT(name != tree->id, "Name has already been given to a node");                   \
        MOT_tree** branch = (name > tree->id) ? &tree->major : &tree->minor;                    \
        if(!(*branch))                                                                          \
        {                                                                                       \
            MOT_ALLOC_TYPE(tag, length, *branch, (v));                                          \
        }                                                                                       \
        else                                                                                    \
        {                                                                                       \
            if(tree->tag != MOT_TAG_BRANCH)                                                     \
                fn(*branch, name, tag, length, value);                                          \
            else                                                                                \
            {                                                                                   \
                struct MOT_node* node;                                                          \
                MOT_ALLOC_TYPE(tag, length, node, (v));                                         \
                ((*branch)->id < name) ? (node->minor = *branch) : (node->major = *branch);     \
                ((*branch)->id < name) ? (node->minor = NULL) : (node->major = NULL);           \
                *branch = node;                                                                 \
            }                                                                                   \
        }                                                                                       \
	} while(0)

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


void _motInsertString(MOT_tree* tree, SPlong name, MOT_tag tag, SPsize length, const SPchar* value)
{
    MOT_INIT_RECURSIVE(_motInsertString, value);
}

void motInsertString(MOT_tree* tree, const SPchar* name, const SPchar* value)
{
	if(value) 
	{
		long long hash = _motHashSDBM(name);
		_motInsertString(tree, hash, MOT_TAG_STRING, strlen(value), value);
	}
}

void motInsertByteArray(MOT_tree* tree, const SPchar* name, MOT_byte_array value)
{
	if(value.data && value.length > 0)
	{
		long long hash = _motHashSDBM(name);
		_motInsertString(tree, hash, MOT_TAG_BYTE & MOT_TAG_ARRAY, value.length * sizeof(SPbyte), value.data);
	}
}
/*<==========================================================>*
 *  debug
 *<==========================================================>*/

const char* _motGetTypeStr(MOT_tag tag)
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
		    MOT_tag tag = (MOT_tag) tree->tag & MOT_ARRAY_MASK;
            switch(tag)
            {
                case MOT_TAG_BYTE:
                case MOT_TAG_SHORT:
                case MOT_TAG_INT:
                case MOT_TAG_LONG:
                {
                    printf("%s array (%lld)\n", _motGetTypeStr(tag), tree->id);
                    for(int i = 0; i < level + 1; i++)
                        printf("\t");

                    SPsize stride = _motBaseLength(tag);
                    for(SPsize i = 0; i < tree->length; i++)
                    {
                        long l = 0;
                        memcpy(&l, tree->buffer + i * stride, get)
                    }
                    break;
                }
            }
		    return;
		}

		switch(tree->tag)
		{
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
				
				const char* type = NULL;
				switch(tree->length)
				{
					case 1: type = "byte"; break;
					case 2: type = "short"; break;
					case 4: type = "int"; break;
					case 8: type = "long"; break;
				}
				printf("%s (%lld): %lld\n", type, tree->id, value);
				break;
			}
			
			case MOT_TAG_FLOAT:
			case MOT_TAG_DOUBLE:
			{
				SPbyte* buffer = tree->data;
				SP_ASSERT(buffer, "Node has invalid data");
				SP_ASSERT(tree->length <= 8 && tree->length >= 4, "Invalid data size for an integer");
				
				switch(tree->length)
				{
					case 4: 
					{
						float value = 0.0f;
						memcpy(&value, buffer, tree->length * sizeof(SPbyte));
						printf("float (%lld): %f\n", tree->id, value);
						break;
					}
					case 8: 
					{
						double value = 0.0;
						memcpy(&value, buffer, tree->length * sizeof(SPbyte));
						printf("double (%lld): %f\n", tree->id, value);
						break;
					}
				}
				
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
		motPrintTreeImpl(tree->major, level + 1);
		motPrintTreeImpl(tree->minor, level + 1);
	}
}
void motPrintTree(const MOT_tree* tree)
{
	motPrintTreeImpl(tree, 0);
	printf("\n");
}