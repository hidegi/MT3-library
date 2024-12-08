/****************************************************************************
 * Copyright (c) 2024 Hidegi
 *
 * This software is provided ‘as-is’, without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ****************************************************************************/
#ifndef MT3_INTERNAL_IMPL_H
#define MT3_INTERNAL_IMPL_H
#include "mt3.h"
#include "platform.h"
#define MT3_CHECKED_CALLOC(ptr, n, size, on_error)\
	do\
	{\
		if(!((ptr) = calloc(n, size)))\
		{\
			errno = MT3_STATUS_NO_MEMORY;\
			on_error;\
		}\
	} while(0)

const SPchar* _mt3_tag_to_str(MT3_tag tag);
MT3_node _mt3_search(const MT3_node tree, const SPchar* name);
SPbool _mt3_is_name_valid(const SPchar* name);
SPint _mt3_strcmp(const SPchar* a, const SPchar* b);
void _mt3_strncpy(SPchar** dst, const SPchar* src, SPsize length);
SPbuffer _mt3_compress(const void* memory, SPsize length);
SPbuffer _mt3_decompress(const void* memory, SPsize length);
void _mt3_encode_tree(const MT3_node tree, SPbuffer* buffer, int level);
void _mt3_encode_list(const MT3_node list, SPbuffer* buffer, int level);
MT3_node _mt3_decode_tree(const SPubyte** memory, SPsize* length);
SPbool _mt3_decode_list(MT3_node node, const SPubyte** memory, SPsize* length);
SPbool _mt3_is_tree_equal(const MT3_node a, const MT3_node b);
SPbool _mt3_is_list_equal(const MT3_node a, const MT3_node b);
SPbool _mt3_is_root(const MT3_node node);
SPbool _mt3_is_major(const MT3_node node);
void _mt3_delete_tree_impl(MT3_node tree);
void _mt3_delete_list_impl(MT3_node tree);
void _mt3_fix_rbt_violations(MT3_node node, MT3_node* head);
void _mt3_delete_node(MT3_node n);
void _mt3_bst_delete_impl(MT3_node n, MT3_node* _r, MT3_node* _x, MT3_node* _w, MT3_node* head);
SPbool _mt3_fix_up_rbt(SPbool redBefore, MT3_node r, MT3_node x, MT3_node w, MT3_node* head);
void _mt3_print_tree(const MT3_node tree, SPbuffer* buffer, int level);
void _mt3_print_list(const MT3_node tree, SPbuffer* buffer, int level);
SPsize _mt3_length_of_list(const MT3_node list);
#endif
