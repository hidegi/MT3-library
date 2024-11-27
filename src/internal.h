#ifndef MT3_INTERNAL_IMPL_H
#define MT3_INTERNAL_IMPL_H
#include "mt3.h"

struct _MT3_node
{
	//written to disk:
	SPhash weight; // 8 bytes
	MT3_tag tag;   // 1 byte
	SPsize length; // 8 bytes

	// encoded in the 7th bit of the tag..
	// signals the node's color..
	SPbool red;

	union
	{
		SPbyte tag_byte;
		SPshort tag_short;
		SPint tag_int;
		SPlong tag_long;
		SPfloat tag_float;
		SPdouble tag_double;
		SPchar* tag_string;
		MT3_node tag_object; // Meant for all list types and trees
	} payload; //length bytes

	// total bytes written = 17 + length bytes..
	struct _MT3_node* parent;
	struct _MT3_node* major;
	struct _MT3_node* minor;
};

typedef struct _MT3_node* MT3_node;

const char* _mt3_tag_to_str(MT3_tag tag);
MT3_node _mt3_search(const MT3_node tree, const char* name);
SPhash _mt3_sdbm(const SPchar* str);

SPbool _mt3_is_root(const MT3_node node);
SPbool _mt3_is_major(const MT3_node node);

void _mt3_fix_rbt_violations(MT3_node node, MT3_node* head);

void _mt3_print_tree(const MT3_node tree, int level);
void _mt3_print_list(const MT3_node tree, int level);
SPsize _mt3_length_of_list(const MT3_node list);
/*
static const char* _mt3_tag_to_str(MT3_tag tag);
static void _mt3_encode(const MT3_node tree, SPbuffer* buffer, int level);
static SPbool _mt3_decode(MT3_node tree, const SPubyte** memory, SPsize* length);
static SPsize _mt3_length_of_array(const MT3_node array);
static void _mt3_insert_array(MT3_node* tree, const SPchar* name, MT3_tag tag, SPsize length, const void* data);
static void _mt3_write_binary(const MT3_node tree, SPbuffer* buffer, int level);
static SPchar* _mt3_init_string_array_buffer(SPsize* bCount, SPsize length, const SPchar** strings);
static MT3_node _mt3_read_binary(const SPubyte** memory, SPsize* length);
static void _mt3_print_tree(const MT3_node tree, int level);
static void _mt3_print_array(const MT3_node array, int level);
*/
#endif