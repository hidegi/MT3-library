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
	MOT_TAG_NONE 	= 0x00,
	MOT_TAG_INTEGER = 0x01,
	MOT_TAG_FLOAT 	= 0x02,
	MOT_TAG_STRING 	= 0x04,
	MOT_TAG_TREE 	= 0x08,
	MOT_TAG_ARRAY	= 0x10
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
	SPsize length;
	SPsize stride;
	SPbyte* data;
} MOT_buffer;

struct MOT_node
{
	SPlong id;
	MOT_tag tag;
	
	union
	{
		MOT_buffer buffer;
		//struct MOT_node* tree;
	} payload;
	struct MOT_node* major;
	struct MOT_node* minor;
};

typedef struct
{
	SPsize length;
	SPbyte* data;
} MOT_byte_array;

typedef struct MOT_node MOT_tree;

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
SP_API void motAddByte(MOT_tree* tree, const SPchar* name, SPbyte value);
SP_API void motAddShort(MOT_tree* tree, const SPchar* name, SPshort value);
SP_API void motAddInt(MOT_tree* tree, const SPchar* name, SPint value);
SP_API void motAddLong(MOT_tree* tree, const SPchar* name, SPlong value);
SP_API void motAddFloat(MOT_tree* tree, const SPchar* name, SPfloat value);
SP_API void motAddDouble(MOT_tree* tree, const SPchar* name, SPdouble value);
SP_API void motAddString(MOT_tree* tree, const SPchar* name, const SPchar* value);
SP_API void motAddByteArray(MOT_tree* tree, const SPchar* name, MOT_byte_array value);


/*<==========================================================>*
 *  freeing
 *<==========================================================>*/
SP_API void motFreeTree(MOT_tree* tree);


//typedef MOTnode MOThead;

#ifdef __cplusplus
}
#endif
#endif