#ifndef SP_MT3REE_H
#define SP_MT3REE_H
#include "SP/config.h"
#include "SP/sparse/buffer.h"

#ifdef __cplusplus
extern "C" {
#endif
 
/*<==========================================================>*
 *  this API provides basic functionality to create BTOs..
 *
 *	however, it does not come with the functionality to create 
 *	multi-tree objects automatically and therefore, richer APIs 
 *	may build upon this simple interface..
 *
 *	this API uses string-names to generate an 8-Byte hash-value,
 *	however, key-generation can be implementation-dependent..
 *	
 *	simple functionalities provided:
 *	--------------------------------
 *	+ inserting nodes with basic data types, arrays and roots..
 *	+ searching for a node..
 *	+ reading from nodes..
 *	+ writing to non-immutable nodes..
 *	+ deleting nodes..
 *	+ reading and writing BTOs to files..
 *	+ self balancing using RBT-scheme..
 *	
 *	immutable nodes are any types of arrays and roots..
 *	lacks the feature to iterate over an ordered set..
 *<==========================================================>*/
typedef enum
{
	MT3_TAG_NULL 	= 0,
	MT3_TAG_ROOT    = 1,
	MT3_TAG_BYTE    = 2,
	MT3_TAG_SHORT   = 3,
	MT3_TAG_INT     = 4,
	MT3_TAG_LONG    = 5,
	MT3_TAG_FLOAT 	= 6,
	MT3_TAG_DOUBLE  = 7,
	MT3_TAG_STRING 	= 8,
	MT3_TAG_ARRAY	= 0x80,

	MT3_TAG_ROOT_ARRAY	 = MT3_TAG_ARRAY | MT3_TAG_ROOT,
	MT3_TAG_BYTE_ARRAY   = MT3_TAG_ARRAY | MT3_TAG_BYTE,
	MT3_TAG_SHORT_ARRAY  = MT3_TAG_ARRAY | MT3_TAG_SHORT,
	MT3_TAG_INT_ARRAY    = MT3_TAG_ARRAY | MT3_TAG_INT,
	MT3_TAG_LONG_ARRAY   = MT3_TAG_ARRAY | MT3_TAG_LONG,
	MT3_TAG_FLOAT_ARRAY  = MT3_TAG_ARRAY | MT3_TAG_FLOAT,
	MT3_TAG_DOUBLE_ARRAY = MT3_TAG_ARRAY | MT3_TAG_DOUBLE,
	MT3_TAG_STRING_ARRAY = MT3_TAG_ARRAY | MT3_TAG_STRING
} MT3_tag;

typedef enum
{
	MT3_STATUS_OK = 0,
	MT3_STATUS_NO_MEMORY,
	MT3_STATUS_WRITE_ERROR,
	MT3_STATUS_READ_ERROR,
	MT3_STATUS_BAD_NAME,
	MT3_STATUS_BAD_VALUE,
	MT3_STATUS_BAD_TAG,
	MT3_STATUS_TYPE_ERROR
} MT3_status;

struct MT3_node;
typedef struct MT3_node* MT3_tree;
typedef struct MT3_node* MT3_array;
typedef struct MT3_node MT3_node;

/*<==========================================================>*
 *  debug
 *<==========================================================>*/
SP_API void mt3_PrintTree(const MT3_tree tree);
SP_API void mt3_PrintArray(const MT3_array tree);

/*<==========================================================>*
 *  allocation
 *<==========================================================>*/
SP_API MT3_tree mt3_AllocObject();
SP_API MT3_array mt3_AllocArray();

/*<==========================================================>*
 *  copying
 *<==========================================================>*/
SP_API MT3_tree mt3_CopyTree(const MT3_tree n);
SP_API MT3_array mt3_CopyArray(const MT3_array n);

/*<==========================================================>*
 *  data feed
 *<==========================================================>*/
SP_API void* mt3_AllocChunk(SPsize size);
SP_API void mt3_InsertByte(MT3_tree* tree, const SPchar* name, SPbyte value);
SP_API void mt3_InsertShort(MT3_tree* tree, const SPchar* name, SPshort value);
SP_API void mt3_InsertInt(MT3_tree* tree, const SPchar* name, SPint value);
SP_API void mt3_InsertLong(MT3_tree* tree, const SPchar* name, SPlong value);
SP_API void mt3_InsertFloat(MT3_tree* tree, const SPchar* name, SPfloat value);
SP_API void mt3_InsertDouble(MT3_tree* tree, const SPchar* name, SPdouble value);
SP_API void mt3_InsertString(MT3_tree* tree, const SPchar* name, const SPchar* value);

/*
 *	insert an empty tree with value = NULL,
 *	otherwise copies a tree and uses it as sub-tree..
 */
SP_API void mt3_InsertTree(MT3_tree* tree, const SPchar* name, MT3_tree value);
SP_API void mt3_InsertByteArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPbyte* values);
SP_API void mt3_InsertShortArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPshort* values);
SP_API void mt3_InsertIntArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPint* values);
SP_API void mt3_InsertLongArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPlong* values);
SP_API void mt3_InsertFloatArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPfloat* values);
SP_API void mt3_InsertDoubleArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPdouble* values);
SP_API void mt3_InsertStringArray(MT3_tree* tree, const SPchar* name, SPsize length, const SPchar** values);
SP_API void mt3_InsertArray(MT3_tree* tree, const SPchar* name, MT3_array array);

SP_API void mt3_ArrayInsertTree(MT3_array* array, MT3_tree value);
SP_API void mt3_ArrayInsertByteArray(MT3_array* array, SPsize length, const SPbyte* values);
SP_API void mt3_ArrayInsertShortArray(MT3_array* array, SPsize length, const SPshort* values);
SP_API void mt3_ArrayInsertIntArray(MT3_array* array, SPsize length, const SPint* values);
SP_API void mt3_ArrayInsertLongArray(MT3_array* array, SPsize length, const SPlong* values);
SP_API void mt3_ArrayInsertFloatArray(MT3_array* array, SPsize length, const SPfloat* values);
SP_API void mt3_ArrayInsertDoubleArray(MT3_array* array, SPsize length, const SPdouble* values);
SP_API void mt3_ArrayInsertStringArray(MT3_array* array, SPsize length, const SPchar** values);
SP_API void mt3_ArrayInsertArray(MT3_array* array, MT3_array list);

SP_API void mt3_SetByte(MT3_tree tree, const char* name, SPbyte value);
SP_API void mt3_SetShort(MT3_tree tree, const char* name, SPshort value);
SP_API void mt3_SetInt(MT3_tree tree, const char* name, SPint value);
SP_API void mt3_SetLong(MT3_tree tree, const char* name, SPlong value);
SP_API void mt3_SetFloat(MT3_tree tree, const char* name, SPfloat value);
SP_API void mt3_SetDouble(MT3_tree tree, const char* name, SPdouble value);
SP_API void mt3_SetString(MT3_tree tree, const char* name, const SPchar* value);

// returns the payload..
SP_API SPbyte mt3_GetByte(const MT3_tree tree, const SPchar* name);
SP_API SPshort mt3_GetShort(const MT3_tree tree, const SPchar* name);
SP_API SPint mt3_GetInt(const MT3_tree tree, const SPchar* name);
SP_API SPlong mt3_GetLong(const MT3_tree tree, const SPchar* name);
SP_API SPfloat mt3_GetFloat(const MT3_tree tree, const SPchar* name);
SP_API SPdouble mt3_GetDouble(const MT3_tree tree, const SPchar* name);
SP_API const SPchar* mt3_GetString(const MT3_tree tree, const SPchar* name);
/*
 *	careful here!!
 *	this returns an actual L-value pointer to a multi-tree..
 *	therefore, do not do something stupid with it.. ;D
 */
SP_API MT3_tree* mt3_GetTree(const MT3_tree tree, const SPchar* name);
SP_API MT3_array mt3_GetArray(const MT3_tree tree, const SPchar* name);

// returns all root nodes..
// intended use for root-nodes that act as a list..
// however, you must free the returned pointer manually..
// (in-order traversal)..
SP_API MT3_tree** mt3_AsList(const MT3_tree tree);

//SP_API MT3_node* mt3_Search(const MT3_tree tree, const char* name);
SP_API SPbool mt3_Delete(MT3_tree* tree, const SPchar* name);
SP_API MT3_tree mt3_CopyTree(const MT3_tree);
SP_API MT3_array mt3_CopyArray(const MT3_array);

SP_API SPbuffer mt3_WriteBinary(const MT3_tree tree);
SP_API MT3_tree mt3_ReadBinary(SPbuffer buffer);

/*<==========================================================>*
 *  freeing
 *<==========================================================>*/
SP_API void mt3_FreeTree(MT3_tree* tree);
SP_API void mt3_FreeArray(MT3_array* tree);

/*<==========================================================>*
 *  check if the tree is a valid RB-tree
 *<==========================================================>*/
SP_API SPbool mt3_VerifyRBT(const MT3_tree rbt);

/*<==========================================================>*
 *  error signaling
 *<==========================================================>*/
SP_API MT3_status mt3_GetLastError();
SP_API const char* mt3_GetErrorInfo(MT3_status status);

#ifdef __cplusplus
}
#endif
#endif
