#ifndef SP_MOTREE_H
#define SP_MOTREE_H
#include "SP/config.h"
#include "SP/sparse/buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	NBT				MOT
 *	byte			byte-sequence (stride 1) length 1
 *	short			byte-sequence (stride 2) length 1
 *	int				byte-sequence (stride 4) length 1
 *	long			byte-sequence (stride 8) length 1
 *	string			byte-sequence (stride 1) length 1
 *	float			byte-sequence (stride 4) length 1
 *	double			byte-sequence (stride 8) length 1
 *	byte array		byte-sequence (stride 1) length n
 *	int array		byte-sequence (stride 4) length n
 *	long array		byte-sequence (stride 8) length n
 *	string array	byte-sequence (stride n) length n
 *	list			tree (order by id)
 *	compound		tree (order by id)
 */
 
typedef enum
{
	MOT_TAG_NULL 	= 0, // NULL
	MOT_TAG_ROOT    = 1, // ROOT
	MOT_TAG_BYTE    = 2, // BYTE
	MOT_TAG_SHORT   = 3, // SHORT
	MOT_TAG_INT     = 4, // INT
	MOT_TAG_LONG    = 5, // LONG
	MOT_TAG_FLOAT 	= 6, // FLOAT
	MOT_TAG_DOUBLE  = 7, // DOUBLE
	MOT_TAG_STRING 	= 8, // STRING	
	MOT_TAG_ARRAY	= 0x80, //ARRAY


	// optimized list of roots without writing tag and weight on disk.. (only applies to roots!!)
	// elements are indexed from 1 to n, since no node can have a weight of 0..
	// in memory, this is a double-linked list, where the major branch points to the next element,
	MOT_TAG_LIST	     = MOT_TAG_ARRAY | MOT_TAG_ROOT,
	MOT_TAG_BYTE_ARRAY   = MOT_TAG_ARRAY | MOT_TAG_BYTE,
	MOT_TAG_SHORT_ARRAY  = MOT_TAG_ARRAY | MOT_TAG_SHORT,
	MOT_TAG_INT_ARRAY    = MOT_TAG_ARRAY | MOT_TAG_INT,
	MOT_TAG_LONG_ARRAY   = MOT_TAG_ARRAY | MOT_TAG_LONG,
	MOT_TAG_FLOAT_ARRAY  = MOT_TAG_ARRAY | MOT_TAG_FLOAT,
	MOT_TAG_DOUBLE_ARRAY = MOT_TAG_ARRAY | MOT_TAG_DOUBLE,
	MOT_TAG_STRING_ARRAY = MOT_TAG_ARRAY | MOT_TAG_STRING
} MOT_tag;

typedef enum
{
	MOT_ERR_NONE,
	MOT_ERR_IO,
	MOT_ERR_MEM,
	MOT_ERR_GEN,
	MOT_ERR_COMP
} MOT_status;

struct MOT_node;
typedef struct MOT_node* MOT_tree;

/*<==========================================================>*
 *  debug
 *<==========================================================>*/
SP_API void motPrintTree(const MOT_tree tree);

/*<==========================================================>*
 *  allocation
 *<==========================================================>*/
SP_API MOT_tree motAllocTree();

/*<==========================================================>*
 *  data feed
 *<==========================================================>*/
SP_API void* motAllocChunk(SPsize size);
SP_API void motInsertByte(MOT_tree* tree, const SPchar* name, SPbyte value);
SP_API void motInsertShort(MOT_tree* tree, const SPchar* name, SPshort value);
SP_API void motInsertInt(MOT_tree* tree, const SPchar* name, SPint value);
SP_API void motInsertLong(MOT_tree* tree, const SPchar* name, SPlong value);
SP_API void motInsertFloat(MOT_tree* tree, const SPchar* name, SPfloat value);
SP_API void motInsertDouble(MOT_tree* tree, const SPchar* name, SPdouble value);
SP_API void motInsertString(MOT_tree* tree, const SPchar* name, const SPchar* value);
SP_API void motInsertTree(MOT_tree* tree, const SPchar* name, MOT_tree value);

SP_API void motInsertArray(MOT_tree* tree, const SPchar* name, MOT_tag tag, SPsize length, const void* value);
SP_API void motInsertStringArray(MOT_tree* tree, const SPchar* name, SPsize length, const SPchar** value);

SP_API MOT_tree motSearch(MOT_tree tree, const char* name);
SP_API SPbool motDelete(MOT_tree tree, const SPchar* name);

SP_API SPbuffer motWriteBinary(const MOT_tree tree);
SP_API MOT_tree motReadBinary(SPbuffer buffer);

/*<==========================================================>*
 *  freeing
 *<==========================================================>*/
SP_API void motFreeTree(MOT_tree tree);


//typedef MOTnode MOThead;

#ifdef __cplusplus
}
#endif
#endif