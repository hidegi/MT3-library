#ifndef SP_MOTREE_H
#define SP_MOTREE_H
#include "SP/config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	NBT				MOT
 *	byte			number (stride 1) length 1
 *	short			number (stride 2) length 1
 *	int				number (stride 4) length 1
 *	long			number (stride 8) length 1
 *	string			number (stride 1) length 1
 *	float			number (stride 4) length 1
 *	double			number (stride 8) length 1
 *	byte array		number (stride 1) length n
 *	int array		number (stride 4) length n
 *	long array		number (stride 8) length n
 *	list			tree (order by id)
 *	compound		tree (order by id)
 */
 
typedef enum
{
	MOT_TAG_NONE 	= 0x0000,
	MOT_TAG_INTEGER = 0x0001,
	MOT_TAG_STRING 	= 0x0002,
	MOT_TAG_FLOAT 	= 0x0004,
	MOT_TAG_TREE 	= 0x0008
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
		struct
		{
			struct MOT_node* left;
			struct MOT_node* right;
		} tree;
	} payload;
};

typedef struct MOT_node MOT_tree;

/*<==========================================================>*
 *  debug
 *<==========================================================>*/
SP_API void motPrintTree(const MOT_tree* tree);

/*<==========================================================>*
 *  allocation
 *<==========================================================>*/
SP_API MOT_tree* motAllocTree();

/*<==========================================================>*
 *  data feed
 *<==========================================================>*/
SP_API void motAddInteger(const SPchar* name, SPint value);

/*<==========================================================>*
 *  freeing
 *<==========================================================>*/
SP_API void motFreeTree(MOT_tree* tree);


//typedef MOTnode MOThead;

#ifdef __cplusplus
}
#endif
#endif