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
#ifndef MT3_PUBLIC_INTERFACE_H
#define MT3_PUBLIC_INTERFACE_H

#include "platform.h"
#include "types.h"
#include "buffer.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	/// Plain-type tags.
	MT3_TAG_NULL 	= 0,
	MT3_TAG_ROOT    = 1,
	MT3_TAG_BYTE    = 2,
	MT3_TAG_SHORT   = 3,
	MT3_TAG_INT     = 4,
	MT3_TAG_LONG    = 5,
	MT3_TAG_FLOAT 	= 6,
	MT3_TAG_DOUBLE  = 7,
	MT3_TAG_STRING 	= 8,
	MT3_TAG_LIST	= 0x80,
	
	/// List-type tags.
	MT3_TAG_ROOT_LIST   = MT3_TAG_LIST | MT3_TAG_ROOT,
	MT3_TAG_BYTE_LIST   = MT3_TAG_LIST | MT3_TAG_BYTE,
	MT3_TAG_SHORT_LIST  = MT3_TAG_LIST | MT3_TAG_SHORT,
	MT3_TAG_INT_LIST    = MT3_TAG_LIST | MT3_TAG_INT,
	MT3_TAG_LONG_LIST   = MT3_TAG_LIST | MT3_TAG_LONG,
	MT3_TAG_FLOAT_LIST  = MT3_TAG_LIST | MT3_TAG_FLOAT,
	MT3_TAG_DOUBLE_LIST = MT3_TAG_LIST | MT3_TAG_DOUBLE,
	MT3_TAG_STRING_LIST = MT3_TAG_LIST | MT3_TAG_STRING
} MT3_tag;

typedef enum
{
	MT3_STATUS_OK = 0, 	/// No errors.
	MT3_STATUS_NO_MEMORY, 	/// Out of memory.
	MT3_STATUS_READ_ERROR,	/// Read error.
	MT3_STATUS_WRITE_ERROR,	/// Write error.
	MT3_STATUS_BAD_NAME,	/// Bad name, either taken, empty or null.
	MT3_STATUS_BAD_VALUE,	/// Invalid value.
	MT3_STATUS_NOT_A_TREE,  /// Given node is not a tree.
	MT3_STATUS_NOT_A_LIST,  /// Given node is not a list.
	MT3_STATUS_BAD_TAG,	/// Invalid tag.
} MT3_status;

struct _MT3_node
{	
	// Do not modify these members directly!
	// (Causes undefined behaviour)
	SPhash weight;
	MT3_tag tag;
	
	SPubyte nameLength; // Name should not exceed 255 characters.
	SPchar* name; // The name of the node.
	
	SPsize length; // The length of the payload in bytes.
	
	SPbool red; // Signals the node's color, encoded in the 7th bit of the tag.
	struct _MT3_node* parent;
	struct _MT3_node* major;
	struct _MT3_node* minor; 
	
	union
	{
		// Payload permitted for direct modification, except 
		// tag_string and tag_object.
		SPbyte tag_byte;
		SPshort tag_short;
		SPint tag_int;
		SPlong tag_long;
		SPfloat tag_float;
		SPdouble tag_double;
		SPchar* tag_string;
		struct _MT3_node* tag_object; // For all list-types and trees.
	} payload;
};
typedef struct _MT3_node* MT3_node;

/*!
 *	@brief Allocates a tree-node.
 *
 *	The returned node will be black-coded and 
 *	will have a tag of MT3_TAG_NULL.
 *	
 *	@return The newly allocated tree, otherwise NULL,
 *	if failing to reserve memory.
 *
 *	@ingroup allocation
 */
SP_API MT3_node mt3_AllocTree();

/*!
 *	@brief Allocates a list-node.
 *
 *	The returned node will be red-coded and 
 *	will have a tag of MT3_TAG_NULL.
 *	
 *	@return The newly allocated list, otherwise NULL,
 *	if failing to reserve memory.
 *
 *	@ingroup allocation
 */
SP_API MT3_node mt3_AllocList();

/*!
 *	@brief Dumps an object to a human-readable string.
 *
 *	For tree-nodes, the node will be shown with either R or B 
 *	for red or black respectively.
 *
 *	A plus '+' shows the major branch, a minus '-' the minor branch,
 *	a tilde '~' shows a root node.
 *
 *	For list-nodes, each element will be shown line by line.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, 
 *	if @param object is not a head-node.
 *	
 *	@return The output string.
 *
 *	@ingroup debug
 */
SP_API const SPchar* mt3_ToString(const MT3_node object);

/*!
 *	@brief Same as @ref mt3_ToString, but prints the output
 *	to the standard output stream.
 *
 *	@ingroup debug
 */
SP_API void mt3_Print(const MT3_node object);

/*!
 *	@brief Compares the tag, colour, value and
 *  	weight between two nodes.
 *
 *	@return True, if @param a is virtually
 *  	equal to @param b.
 *
 *	@ingroup comparison
 */
SP_API SPbool mt3_IsEqual(const MT3_node a, const MT3_node b);

/*!
 *	@brief Copies a tree-or list-node.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, 
 *	if @param object is not a head-node.
 *	
 *	@return The newly copied node.
 *	
 *	@ingroup copying
 */
SP_API MT3_node mt3_Copy(const MT3_node object);

/*!
 *	@brief Tells whether or not @param tree is a tree-node.
 *	A tree is defined to be black-coded without a parent.
 *
 *	Expects @param object to be a head-node, otherwise will
 *	always return false.
 *
 *	@return True, if @param node is a tree-node.
 *	
 *	@ingroup tree-validation
 */
SP_API SPbool mt3_IsTree(const MT3_node tree);

/*!
 *	@brief Inserts a tree-or list-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *	 
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value
 *	is neither a tree-nor a list-node or NULL.
 *	
 *	@ingroup tree-insertion
 */
SP_API void mt3_Insert(MT3_node* tree, const char* name, const MT3_node value);

/*!
 *	@brief Inserts a byte-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertByte(MT3_node* tree, const char* name, SPbyte value);

/*!
 *	@brief Inserts a short-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertShort(MT3_node* tree, const char* name, SPshort value);

/*!
 *	@brief Inserts an int-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *	
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertInt(MT3_node* tree, const char* name, SPint value);

/*!
 *	@brief Inserts a long-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertLong(MT3_node* tree, const char* name, SPlong value);

/*!
 *	@brief Inserts a float-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertFloat(MT3_node* tree, const char* name, SPfloat value);

/*!
 *	@brief Inserts a double-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertDouble(MT3_node* tree, const char* name, SPdouble value);

/*!
 *	@brief Inserts a string-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL.
 *	
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertString(MT3_node* tree, const char* name, const SPchar* value);

/*!
 *	@brief Inserts a byte-list-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *	A linked list will be implicitly created from @param values.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertByteList(MT3_node* tree, const char* name, SPsize length, const SPbyte* values);

/*!
 *	@brief Inserts a short-list-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *	A linked list will be implicitly created from @param values.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertShortList(MT3_node* tree, const char* name, SPsize length, const SPshort* values);

/*!
 *	@brief Inserts an int-list-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertIntList(MT3_node* tree, const char* name, SPsize length, const SPint* values);

/*!
 *	@brief Inserts a long-list-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *	A linked list will be implicitly created from @param values.
 * 
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertLongList(MT3_node* tree, const char* name, SPsize length, const SPlong* values);

/*!
 *	@brief Inserts a float-list-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *	A linked list will be implicitly created from @param values.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertFloatList(MT3_node* tree, const char* name, SPsize length, const SPfloat* values);

/*!
 *	@brief Inserts a double-list-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *	A linked list will be implicitly created from @param values.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertDoubleList(MT3_node* tree, const char* name, SPsize length, const SPdouble* values);

/*!
 *	@brief Inserts a string-list-node to the given tree.
 *	If @param tree points to NULL, a new tree will be created.
 *	A linked list will be implicitly created from @param values.
 *	Note that NULL-strings will be ignored.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup tree-insertion
 */
SP_API void mt3_InsertStringList(MT3_node* tree, const char* name, SPsize length, const SPchar** values);

/*!
 *	@brief Returns the value stored in any integer-type nodes
 *	found by @param name.
 *
 *	@return Returns 0LL, if @param tree is not a tree or
 *	the node labeled by @param name is not available,
  *	otherwise the value stored in the node labeled by 
 *	@param name.
 *
 *	@ingroup tree-getters
 */
SP_API SPlong mt3_GetNumber(const MT3_node tree, const SPchar* name);

/*!
 *	@brief Returns the value stored in any floating-type nodes
 *	found by @param name.
 *
 *	@return Returns 0.0, if @param tree is not a tree or
 *	the node labeled by @param name is not available,
 *	otherwise the value stored in the node labeled by 
 *	@param name.
 *
 *	@ingroup tree-getters
 */
SP_API SPdouble mt3_GetDecimal(const MT3_node tree, const SPchar* name);

/*!
 *	@brief Returns the value stored in a byte-node
 *	found by @param name.
 *
 *	@return Returns 0, if @param tree is not a tree or
 *	the node labeled by @param name is not available,
 *	otherwise the value stored in the node labeled by 
 *	@param name.
 *
 *	@ingroup tree-getters
 */
SP_API SPbyte mt3_GetByte(const MT3_node tree, const SPchar* name);

/*!
 *	@brief Returns the value stored in a short-node
 *	found by @param name.
 *
 *	@return Returns 0, if @param tree is not a tree or
 *	the node labeled by @param name is not available,
 *	otherwise the value stored in the node labeled by 
 *	@param name.
 *
 *	@ingroup tree-getters
 */
SP_API SPshort mt3_GetShort(const MT3_node tree, const SPchar* name);

/*!
 *	@brief Returns the value stored in an int-node
 *	found by @param name.
 *
 *	@return Returns 0, if @param tree is not a tree or
 *	the node labeled by @param name is not available,
 *	otherwise the value stored in the node labeled by 
 *	@param name.
 *
 *	@ingroup tree-getters
 */
SP_API SPint mt3_GetInt(const MT3_node tree, const SPchar* name);

/*!
 *	@brief Returns the value stored in a long-node
 *	found by @param name.
 *
 *	@return Returns 0LL, if @param tree is not a tree or
 *	the node labeled by @param name is not available,
 *	otherwise the value stored in the node labeled by 
 *	@param name.
 *
 *	@ingroup tree-getters
 */
SP_API SPlong mt3_GetLong(const MT3_node tree, const SPchar* name);

/*!
 *	@brief Returns the value stored in a float-node
 *	found by @param name.
 *
 *	@return Returns 0.0, if @param tree is not a tree or
 *	the node labeled by @param name is not available,
 *	otherwise the value stored in the node labeled by 
 *	@param name.
 *
 *	@ingroup tree-getters
 */
SP_API SPfloat mt3_GetFloat(const MT3_node tree, const SPchar* name);

/*!
 *	@brief Returns the value stored in a double-node
 *	found by @param name.
 *
 *	@return Returns 0.0, if @param tree is not a tree or
 *	the node labeled by @param name is not available,
 *	otherwise the value stored in the node labeled by 
 *	@param name.
 *
 *	@ingroup tree-getters
 */
SP_API SPdouble mt3_GetDouble(const MT3_node tree, const SPchar* name);

/*!
 *	@brief Returns the value stored in a string-node
 *	found by @param name.
 *
 *	@return Returns NULL, if @param tree is not a tree or
 *	the node labeled by @param name is not available,
 *	otherwise the value stored in the node labeled by 
 *	@param name.
 *
 *	@ingroup tree-getters
 */
SP_API const SPchar* mt3_GetString(const MT3_node tree, const SPchar* name);

/*!
 *	@brief Returns the object stored in a tree-or list-node.
 *	found by @param name.
 *
 *	@return Returns NULL, if @param tree is not a tree or
 *	the node labeled by @param name is not available,
 *	otherwise the object stored in the node labeled by 
 *	@param name.
 *
 *	@ingroup tree-getters
 */
SP_API MT3_node* mt3_Get(const MT3_node tree, const SPchar* name);

/*!
 *	@brief Sets the value stored in a byte-node found by @param name.
 *	@ingroup tree-setters
 */
SP_API void mt3_SetByte(MT3_node tree, const char* name, SPbyte value);

/*!
 *	@brief Sets the value stored in a short-node found by @param name.
 *	@ingroup tree-setters
 */
SP_API void mt3_SetShort(MT3_node tree, const char* name, SPshort value);

/*!
 *	@brief Sets the value stored in an int-node found by @param name.
 *	@ingroup tree-setters
 */
SP_API void mt3_SetInt(MT3_node tree, const char* name, SPint value);

/*!
 *	@brief Sets the value stored in a long-node found by @param name.
 *	@ingroup tree-setters
 */
SP_API void mt3_SetLong(MT3_node tree, const char* name, SPlong value);

/*!
 *	@brief Sets the value stored in a float-node found by @param name.
 *	@ingroup tree-setters
 */
SP_API void mt3_SetFloat(MT3_node tree, const char* name, SPfloat value);

/*!
 *	@brief Sets the value stored in a double-node found by @param name.
 *	@ingroup tree-setters
 */
SP_API void mt3_SetDouble(MT3_node tree, const char* name, SPdouble value);

/*!
 *	@brief Sets the value stored in a long-node found by @param name.
 *
 *	Issues @ref MT3_STATUS_NO_MEMORY, if failing to copy from @param value.
 *
 *	@ingroup tree-setters
 */
SP_API void mt3_SetString(MT3_node tree, const char* name, const SPchar* value);

/*!
 *	@brief Removes a node from a tree.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param tree is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param tree is not NULL and not a tree.
 *
 *	@ingroup tree-removal
 */
SP_API SPbool mt3_Remove(MT3_node* tree, const SPchar* name);

/*!
 *	@brief Checks if @param rbt conforms the properties
 *	of a red-black-tree.
 *
 *	@return True, if @param rbt conforms the properties
 *	of a red-black-tree.
 *
 *	@ingroup tree-validation
 */
 
SP_API SPbool mt3_IsValidRBT(const MT3_node rbt);

/*!
 *	@brief Tells whether or not @param list is a list-node.
 *	A list is defined to be red-coded without a parent.
 *
 *	@return True, if @param node is a list-node.
 *	
 *	@ingroup list-validation
 */
SP_API SPbool mt3_IsList(const MT3_node list);

/*!
 *	@brief Returns the length of @param list.
 *
 *	@return The amount of elements contained in @param list,
 *	otherwise 0, if @param list is NULL or not a list-node.
 *	
 *	@ingroup list-insertion
 */
SP_API SPsize mt3_Length(const MT3_node list);

/*!
 *	@brief Appends a tree-or list-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	 
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_LIST, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value
 *	is neither a tree-nor a list-node or NULL.
 *	
 *	@ingroup list-insertion
 */
SP_API void mt3_Append(MT3_node* list, const MT3_node value);

/*!
 *	@brief Appends a byte-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendByte(MT3_node* list, SPbyte value);

/*!
 *	@brief Appends a short-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendShort(MT3_node* list, SPshort value);

/*!
 *	@brief Appends an int-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendInt(MT3_node* list, SPint value);

/*!
 *	@brief Appends a long-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendLong(MT3_node* list, SPlong value);

/*!
 *	@brief Appends a float-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendFloat(MT3_node* list, SPfloat value);

/*!
 *	@brief Appends a double-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendDouble(MT3_node* list, SPdouble value);

/*!
 *	@brief Appends a string-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendString(MT3_node* list, const SPchar* value);

/*!
 *	@brief Appends a byte-list-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	A linked list will be implicitly created from @param values.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_LIST, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendByteList(MT3_node* list, SPsize length, const SPbyte* values);

/*!
 *	@brief Appends a short-list-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	A linked list will be implicitly created from @param values.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_LIST, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendShortList(MT3_node* list, SPsize length, const SPshort* values);

/*!
 *	@brief Appends an int-list-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	A linked list will be implicitly created from @param values.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_LIST, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendIntList(MT3_node* list, SPsize length, const SPint* values);

/*!
 *	@brief Appends a long-list-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	A linked list will be implicitly created from @param values.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_LIST, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendLongList(MT3_node* list, SPsize length, const SPlong* values);

/*!
 *	@brief Appends a float-list-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	A linked list will be implicitly created from @param values.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_LIST, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendFloatList(MT3_node* list, SPsize length, const SPfloat* values);

/*!
 *	@brief Appends a double-list-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	A linked list will be implicitly created from @param values.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_LIST, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendDoubleList(MT3_node* list, SPsize length, const SPdouble* values);

/*!
 *	@brief Appends a string-list-node to the given list.
 *	If @param list points to NULL, a new list will be created.
 *	A linked list will be implicitly created from @param values.
 *	Note that NULL-strings will be ignored.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_LIST, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	Issues @ref MT3_STATUS_BAD_NAME, if @param name is NULL,
 *	empty or already taken.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param value is NULL,
 *	or @param length is less-equal to zero.
 *
 *	@ingroup list-insertion
 */
SP_API void mt3_AppendStringList(MT3_node* list, SPsize length, const SPchar** values);

/*!
 *	@brief Converts a contiguous array in memory to a linked list.
 *	
 *	@return NULL, if @param tag was MT3_TAG_NULL, MT3_TAG_ROOT or MT3_TAG_LIST,
 *	otherwise the newly created list.
 *
 *	@ingroup list-conversion
 */
SP_API MT3_node mt3_ToList(MT3_tag tag, SPsize length, const void* data);

/*!
 *	@brief Removes an element of a list at the index @param pos.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param list is NULL.
 *
 *	Issues @ref MT3_STATUS_NOT_A_LIST, if the object pointed
 *	to by @param list is not NULL and not a list.
 *
 *	Issues @ref MT3_STATUS_BAD_VALUE, @param length is less zero
 *	or exceeds the length of @param list.
 *	
 *	@ingroup list-removal
 */
SP_API void mt3_RemoveAt(MT3_node* list, SPindex pos);

/*!
 *	@brief Dumps a tree to binary.
 *	
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if @param is not a tree.
 *
 *	Issues @ref MT3_STATUS_WRITE_ERROR at compression failure.
 *	
 *	@return The newly created byte-array.
 *
 *	@ingroup tree-serialization
 */
SP_API SPbuffer mt3_EncodeTree(const MT3_node tree);

/*!
 *	@brief Dumps a tree to binary.
 *	
 *	Issues @ref MT3_STATUS_NOT_A_TREE, if @param is not a tree.
 *
 *	Issues @ref MT3_STATUS_WRITE_ERROR at decompression failure.
 *	
 *	@return The deserialized tree.
 *
 *	@ingroup tree-deserialization
 */
SP_API MT3_node mt3_DecodeTree(SPbuffer buffer);

/*!
 *	@brief Deletes an object node.
 *	
 *	Issues @ref MT3_STATUS_BAD_VALUE, if @param object
 *	is neither a tree-nor a list-node.
 *
 *	@ingroup deletion
 */
SP_API void mt3_Delete(MT3_node* object);

/*!
 *	@brief Returns the last error and resets to MT3_STATUS_OK.
 *	
 *	@return The last error-code.
 *
 *	@ingroup error-signaling
 */
SP_API MT3_status mt3_GetLastError();

/*!
 *	@brief Returns an informative error-string by @param status.
 *	
 *	@return The error-string
 *
 *	@ingroup error-signaling
 */
SP_API const char* mt3_GetErrorInfo(MT3_status status);
#ifdef __cplusplus
}
#endif
#endif
