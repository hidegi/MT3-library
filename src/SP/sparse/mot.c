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
#define MOT_SCALAR_MASK 0x01FF
#define MOT_INTEGER_MASK 0x003C
#define MOT_FLOAT_MASK 0x00C0
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

struct MOT_node* _motAllocNode(MOT_tag tag, SPlong name, SPsize length, const SPbyte* value)
{
		struct MOT_node* node = NULL;
	    MOT_CHECKED_CALLOC(node, 1, sizeof(struct MOT_node), return NULL);
		node->tag = tag;
		node->id = name;
		node->length = length;
		node->major = node->minor = NULL;

        if(value && length > 0)
        {
            MOT_CHECKED_CALLOC(node->data, node->length, sizeof(SPbyte), return NULL);
            memcpy(node->data, value, node->length * sizeof(SPbyte));
        }

        return node;
}

SPsize _motLengthOf(MOT_tag tag)
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

MOT_tree* _motSearch(MOT_tree* tree, SPlong hash)
{
	if(tree)
	{
		if(tree->id == hash)
			return tree;
			
		MOT_tree* node = NULL;
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

void _motSwapPtr(void** a, void** b)
{
    void* tmp = *a;
    *a = *b;
    *b = tmp;
}

#define MOT_SWAP_PTR(a, b, type)\
    do                          \
    {                           \
        type* tmp = a;          \
        a = b;                  \
        b = tmp;                \
    } while(0);
    
void _motInsertBytes(MOT_tree* tree, SPlong name, MOT_tag tag, SPsize length, const SPbyte* value)
{
    SP_ASSERT(tree, "Cannot Insert integer to empty tree");
    SP_ASSERT(name != tree->id, "Name has already been given to a node");
    SP_ASSERT(tree->id != MOT_TAG_BRANCH && tree->id != MOT_TAG_ROOT, "Cannot insert data to root-or branch-nodes");

    SPbool isMajor = (name > tree->id);
    MOT_tree* primary  = isMajor ? tree->major : tree->minor;
    if(!(primary))
    {
        primary = _motAllocNode(tag, name, length, value);
        isMajor ? (tree->major = primary) : (tree->minor = primary);
    }
    else
    {
        if(tree->tag != MOT_TAG_BRANCH)
            _motInsertBytes(primary, name, tag, length, value);
        else
        {

            SP_ASSERT(!tree->data && tree->length == 0, "Branch nodes cannot hold memory");

            //allocate a new node..
            struct MOT_node* node;
            node = _motAllocNode(tag, name, length, value);
            //swap new node with the branch node..
            tree->tag = node->tag;
            node->tag = MOT_TAG_BRANCH;
           
            node->id = tree->id;
            tree->id = name;
            
            MOT_SWAP_PTR(tree->data, node->data, SPbyte);
            MOT_SWAP_PTR(tree->major, node->major, struct MOT_node);
            MOT_SWAP_PTR(tree->minor, node->minor, struct MOT_node);
            isMajor ? (tree->major = node) : (tree->minor = node);
        }
    }
}

void _motInsertTree(MOT_tree* tree, MOT_tree* value)
{
    if(!tree || tree->tag == MOT_TAG_NULL)
        return;
    SP_ASSERT(value->id != tree->id, "Name has already been given to a node");
    SPbool isMajor = (value->id > tree->id);
    MOT_tree* primary  = isMajor ? tree->major : tree->minor;
    
	if(!primary)
	{
	    isMajor ? (tree->major = value) : (tree->minor = value);
	}
	else
	{
	    if(primary->id != 0)
	    {
            if(tree->tag != MOT_TAG_BRANCH)
                _motInsertTree(primary, value);
            else
            {
                SP_ASSERT(!tree->data && tree->length == 0, "Branch nodes cannot hold memory");
                //create a new node with id 0
                //this will morph into the current node (tree)..

                struct MOT_node* newTree;
                newTree = _motAllocNode(MOT_TAG_BRANCH, tree->id, 0, NULL);
                newTree->major = tree->major;
                newTree->minor = tree->minor;

                tree->id = 0;
                tree->tag = MOT_TAG_NULL;

                tree->major = (tree->id > value->id) ? newTree : value;
                tree->minor = (tree->id > value->id) ? value : newTree;
            }
        }
	}
}

void motInsertTree(MOT_tree* tree, MOT_tree* value)
{
	SP_ASSERT(value, "Cannot insert empty tree");
	SP_ASSERT(value->tag == MOT_TAG_ROOT, "Invalid tag for root-node");
	SP_ASSERT(!value->data, "Root-nodes must not contain data");
	value->tag = MOT_TAG_BRANCH;
	_motInsertTree(tree, value);
}

void motInsertByte(MOT_tree* tree, const SPchar* name, SPbyte value)
{
	long long hash = _motHashSDBM(name);
	_motInsertBytes(tree, hash, MOT_TAG_BYTE, sizeof(SPint), (const SPbyte*) &value);
}

void motInsertShort(MOT_tree* tree, const SPchar* name, SPshort value)
{
	long long hash = _motHashSDBM(name);
	_motInsertBytes(tree, hash, MOT_TAG_SHORT, sizeof(SPshort), (const SPbyte*) &value);
}

void motInsertInt(MOT_tree* tree, const SPchar* name, SPint value)
{
	long long hash = _motHashSDBM(name);
	_motInsertBytes(tree, hash, MOT_TAG_INT, sizeof(SPint), (const SPbyte*) &value);
}

void motInsertLong(MOT_tree* tree, const SPchar* name, SPlong value)
{
	long long hash = _motHashSDBM(name);
	_motInsertBytes(tree, hash, MOT_TAG_LONG, sizeof(SPlong), (const SPbyte*) &value);
}

void motInsertFloat(MOT_tree* tree, const SPchar* name, SPfloat value)
{
	long long hash = _motHashSDBM(name);
	_motInsertBytes(tree, hash, MOT_TAG_FLOAT, sizeof(SPfloat), (const SPbyte*) &value);
}

void motInsertDouble(MOT_tree* tree, const SPchar* name, SPdouble value)
{
	long long hash = _motHashSDBM(name);
	_motInsertBytes(tree, hash, MOT_TAG_DOUBLE, sizeof(SPdouble), (const SPbyte*) &value);
}

void motInsertString(MOT_tree* tree, const SPchar* name, const SPchar* value)
{
	if(value) 
	{
		long long hash = _motHashSDBM(name);
		_motInsertBytes(tree, hash, MOT_TAG_STRING, strlen(value), (const SPbyte*)value);
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

void motInsertArray(MOT_tree* tree, const SPchar* name, MOT_tag tag, SPsize length, const void* data)
{
	if(tag == MOT_TAG_ROOT || tag == MOT_TAG_BRANCH)
	{
		SP_WARNING("Tag \"MOT_TAG_TREE\" or \"MOT_TAG_BRANCH\" invalid with \"MOT_TAG_ARRAY\"");
		return;
	}
	
	if(data && length > 0)
	{
		long long hash = _motHashSDBM(name);
		_motInsertBytes(tree, hash, tag | MOT_TAG_ARRAY, length * _motLengthOf(tag), (const SPbyte*)data);
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
        case MOT_TAG_BRANCH: return "branch";
    }

    return "null";
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
			SPsize stride = _motLengthOf(tree->tag & ~MOT_TAG_ARRAY);
			
			printf("%s (length: %lld stride: %lld) (%lld):\n", _motGetTypeStr(tree->tag), tree->length, stride, tree->id);
			
			for(int i = 0; i < level + 1; i++)
				printf("\t");
			
			if(MOT_INTEGER_MASK & scalar)
			{
				for(SPsize i = 0; i < tree->length; i += stride)
				{
					SPlong l = 0;
					memcpy(&l, tree->data + i, stride * sizeof(SPbyte));
					printf("%lld ", l);
				}
				printf("\n");
			}
			
			if(MOT_FLOAT_MASK & scalar)
			{
				union
				{
					float _f;
					double _d;
				} value;
				for(SPsize i = 0; i < tree->length; i += stride)
				{
					(scalar == MOT_TAG_FLOAT) ? (value._f = 0.f) : (value._d = 0.0);
					memcpy(((scalar == MOT_TAG_FLOAT) ? (void*)&value._f : (void*)&value._d), tree->data + i, stride);
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
			        printf("Connector (%lld):\n", tree->id);
			        break;
			    }
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
					
					SPlong value = 0;
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

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic pop
#endif