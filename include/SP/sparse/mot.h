#ifndef SP_MOTREE_H
#define SP_MOTREE_H
#include "SP/config.h"

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
	MOT_TAG_NULL 	= 0x0000,
	MOT_TAG_ROOT    = 0x0001,
	MOT_TAG_BRANCH 	= 0x0002,
	MOT_TAG_BYTE    = 0x0004,
	MOT_TAG_SHORT   = 0x0008,
	MOT_TAG_INT     = 0x0010,
	MOT_TAG_LONG    = 0x0020,
	MOT_TAG_FLOAT 	= 0x0040,
	MOT_TAG_DOUBLE  = 0x0080,
	MOT_TAG_STRING 	= 0x0100,
	MOT_TAG_ARRAY	= 0x0200
} MOT_tag;

typedef enum
{
	MOT_ERR_NONE,
	MOT_ERR_IO,
	MOT_ERR_MEM,
	MOT_ERR_GEN,
	MOT_ERR_COMP
} MOT_status;

typedef struct
{

} MOT_buffer;

struct MOT_node
{
	SPlong id;
	MOT_tag tag;
	SPsize length;
	SPbyte* data;
	struct MOT_node* major;
	struct MOT_node* minor;
};

typedef struct
{
	SPsize length;
	SPbyte* data;
} MOT_byte_array;

typedef struct
{
	SPsize length;
	SPint* data;
} MOT_int_array;

typedef struct
{
	SPsize length;
	SPlong* data;
} MOT_long_array;

typedef struct MOT_node MOT_tree;
typedef struct MOT_node MOT_node;

/*<==========================================================>*
 *  debug
 *<==========================================================>*/
SP_API void motPrintTree(const MOT_tree* tree);

/*<==========================================================>*
 *  allocation
 *<==========================================================>*/
SP_API MOT_tree* motAllocTree(const SPchar* name);

/*<==========================================================>*
 *  data feed
 *<==========================================================>*/
SP_API void motInsertByte(MOT_tree* tree, const SPchar* name, SPbyte value);
SP_API void motInsertShort(MOT_tree* tree, const SPchar* name, SPshort value);
SP_API void motInsertInt(MOT_tree* tree, const SPchar* name, SPint value);
SP_API void motInsertLong(MOT_tree* tree, const SPchar* name, SPlong value);
SP_API void motInsertFloat(MOT_tree* tree, const SPchar* name, SPfloat value);
SP_API void motInsertDouble(MOT_tree* tree, const SPchar* name, SPdouble value);
SP_API void motInsertString(MOT_tree* tree, const SPchar* name, const SPchar* value);
SP_API void motInsertByteArray(MOT_tree* tree, const SPchar* name, MOT_byte_array value);
SP_API void motInsertIntArray(MOT_tree* tree, const SPchar* name, MOT_int_array value);
SP_API void motInsertLongArray(MOT_tree* tree, const SPchar* name, MOT_long_array value);
SP_API MOT_tree* motSearch(MOT_tree* tree, const char* name);
SP_API MOT_byte_array motAllocByteArray(SPsize length);
SP_API MOT_int_array motAllocIntArray(SPsize length);
SP_API MOT_long_array motAllocLongArray(SPsize length);


/*<==========================================================>*
 *  freeing
 *<==========================================================>*/
SP_API void motFreeTree(MOT_tree* tree);


//typedef MOTnode MOThead;

#ifdef __cplusplus
}
#endif
#endif