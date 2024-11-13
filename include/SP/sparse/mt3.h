#ifndef SP_MT3REE_H
#define SP_MT3REE_H
#include "SP/config.h"
#include "SP/sparse/buffer.h"

#ifdef __cplusplus
extern "C" {
#endif
 
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

	// optimized list of roots without writing tag and weight on disk.. (only applies to roots!!)
	// elements are indexed from 1 to n, since no node can have a weight of 0..
	// in memory, this is a double-linked list, where the major branch points to the next element,
	MT3_TAG_LIST	     = MT3_TAG_ARRAY | MT3_TAG_ROOT,
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
	MT3_STATUS_COMPRESSION_ERROR,
	MT3_STATUS_DECOMPRESSION_ERROR,
    MT3_STATUS_INVALID_NAME,
    MT3_STATUS_INVALID_VALUE,
    MT3_STATUS_INVALID_TAG
} MT3_status;

struct MT3_node;
typedef struct MT3_node* MT3_tree;

/*<==========================================================>*
 *  debug
 *<==========================================================>*/
SP_API void mt3_PrintTree(const MT3_tree tree);

/*<==========================================================>*
 *  allocation
 *<==========================================================>*/
SP_API MT3_tree mt3_AllocTree();

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
SP_API void mt3_InsertTree(MT3_tree* tree, const SPchar* name, MT3_tree value);
SP_API void mt3_InsertArray(MT3_tree* tree, const SPchar* name, MT3_tag tag, SPsize length, const void* value);

SP_API MT3_tree mt3_Search(const MT3_tree tree, const char* name);
SP_API SPbool mt3_Delete(MT3_tree* tree, const SPchar* name);

SP_API SPbuffer mt3_WriteBinary(const MT3_tree tree);
SP_API MT3_tree mt3_ReadBinary(SPbuffer buffer);

/*<==========================================================>*
 *  freeing
 *<==========================================================>*/
SP_API void mt3_FreeTree(MT3_tree* tree);

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