#include "mt3.h"
#include "internal.h"

#define MT3_HAVE_BST_MAJOR_INCLINED
#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wformat"
#endif
#define MT3_CHECKED_CALLOC(ptr, n, size, on_error)		\
	do							\
	{							\
		if(!((ptr) = calloc(n, size)))			\
		{						\
			errno = MT3_STATUS_NO_MEMORY;		\
			on_error;				\
		}						\
	} while(0)

#define MT3_CHECK_INPUT(_name)					\
	SPhash hash = 0;					\
	do							\
	{							\
	    	if(!tree)\
	    	{                                           	\
	        	errno = MT3_STATUS_BAD_VALUE;		\
	       		return;                                 \
	    	}                                           	\
			else\
			{\
				if((*tree) && ((*tree)->parent || (*tree)->red))\
				{\
					errno = MT3_STATUS_BAD_VALUE;\
					return;\
				}\
			}\
	    	if(!_name)                                  	\
	    	{                                           	\
	        	errno = MT3_STATUS_BAD_NAME;		\
	        	return;                         	\
		}                                       	\
		hash = _mt3_sdbm((_name));			\
		if(hash == 0)                           	\
		{                                       	\
		    errno = MT3_STATUS_BAD_NAME;    		\
		    return;                             	\
		}                                       	\
	} while(0)

#define MT3_READ_GENERIC(dst, n, scanner, fail)     		\
	do							\
	{							\
		if(*length < (n))				\
		{						\
			errno = MT3_STATUS_READ_ERROR;\
			fail;					\
		}						\
		*memory = scanner((dst), *memory, (n));		\
		*length -= n;					\
	} while(0)

#define MT3_FOR_EACH(node, cursor)\
	for((cursor) = (node); (cursor) != NULL; (cursor) = (cursor)->major)
		
#define MT3_MAX(a, b) ((a) > (b) ? (a) : (b))
#define ne2be _mt3_big_endian_to_native_endian
#define be2ne _mt3_big_endian_to_native_endian

SPbool mt3_IsTree(const MT3_node node)
{
	SPbool ret = SP_FALSE;
	if(node)
	{
		ret = (node->parent == NULL) && !node->red;
	}
	return ret;
}

SPbool mt3_IsList(const MT3_node node)
{
	SPbool ret = SP_FALSE;
	if(node)
	{
		ret = (node->parent == NULL) && node->red && ((node->tag & MT3_TAG_LIST) != 0);
	}
	return ret;
}

static MT3_node _mt3_alloc_node(MT3_tag tag, SPhash name, SPsize length, const void* value, SPbool copyValue)
{
	MT3_node node = NULL;
	MT3_CHECKED_CALLOC(node, 1, sizeof(struct _MT3_node), return NULL);
	node->tag = tag;
	node->weight = name;
	node->length = length;
	node->major = node->minor = NULL;
	node->parent = NULL;
	node->red = SP_FALSE;
	
	switch(tag)
	{
		case MT3_TAG_NULL: break;
		case MT3_TAG_BYTE: node->payload.tag_byte = *(const SPbyte*) value; break;
		case MT3_TAG_SHORT: node->payload.tag_short = *(const SPshort*) value; break;
		case MT3_TAG_INT: node->payload.tag_int = *(const SPint*) value; break;
		case MT3_TAG_LONG: node->payload.tag_long = *(const SPlong*) value; break;
		case MT3_TAG_FLOAT: node->payload.tag_float = *(const SPfloat*) value; break;
		case MT3_TAG_DOUBLE: node->payload.tag_double = *(const SPdouble*) value; break;
		case MT3_TAG_STRING:
		{
			MT3_CHECKED_CALLOC(node->payload.tag_string, node->length, sizeof(SPbyte), return NULL);
			memcpy(node->payload.tag_string, (const SPbyte*) value, node->length * sizeof(SPbyte));
			node->payload.tag_string[node->length - 1] = 0;
			break;
		}
		
		default:
		{
			node->payload.tag_object = copyValue ? mt3_Copy((const MT3_node) value) : (MT3_node) value; 
			break;
		}
	}
	return node;
}

static SPsize _mt3_length_of(MT3_tag tag)
{
	switch(tag)
	{
		case MT3_TAG_BYTE: return sizeof(SPbyte);
		case MT3_TAG_SHORT: return sizeof(SPshort);
		case MT3_TAG_INT: return sizeof(SPint);
		case MT3_TAG_LONG: return sizeof(SPlong);
		case MT3_TAG_FLOAT: return sizeof(SPfloat);
		case MT3_TAG_DOUBLE: return sizeof(SPdouble);
		case MT3_TAG_NULL: break;
	}
	return 0;
}

static SPchar* _mt3_init_string_list_buffer(SPsize* bCount, SPsize length, const SPchar** strings)
{
	SPchar* memory = NULL;
	if(strings)
	{
		SPsize byteCount = length * (sizeof(SPlong) + 1) + 1;
		for(SPsize i = 0; i < length; i++)
		{
			const SPchar* string = strings[i];
			SP_ASSERT(string, "Cannot copy from NULL-string");
			byteCount += strlen(string);
		}

		
		MT3_CHECKED_CALLOC(memory, byteCount, sizeof(SPbyte), return NULL);

		SPsize counter = 0, i = 0;
		while(i < byteCount - 1)
		{
			SPsize charsToCopy = strlen(strings[counter]) + 1;
			memcpy(memory + i, &charsToCopy, sizeof(SPlong));
			i += sizeof(SPlong);

			memcpy(memory + i, strings[counter], charsToCopy - 1);
			i += (charsToCopy - 1);

			SPbyte zero = 0;
			memcpy(memory + i, &zero, sizeof(SPbyte));
			i += sizeof(SPbyte);
			counter++;
		}
		memory[byteCount - 1] = 0;
		if(bCount)
			*bCount = byteCount;
	}
	return memory;
}

MT3_node mt3_AllocTree()
{
	MT3_node root = NULL;
	MT3_CHECKED_CALLOC(root, 1, sizeof(struct _MT3_node), return NULL);
	root->length = 0LL;
	root->weight = 0LL;
	root->tag = MT3_TAG_NULL;
	root->parent = root->major = root->minor = NULL;
	root->red = SP_FALSE;
	return root;
}

MT3_node mt3_AllocList()
{
	MT3_node list = mt3_AllocTree();
	list->red = SP_TRUE;
	return list;
}

static SPbool _mt3_copy_payload_from_node(const MT3_node src, const MT3_node dst)
{
	if(src && dst)
	{
		switch(dst->tag)
		{
			case MT3_TAG_NULL: break;
			case MT3_TAG_BYTE: dst->payload.tag_byte = src->payload.tag_byte; break;
			case MT3_TAG_SHORT: dst->payload.tag_short = src->payload.tag_short; break;
			case MT3_TAG_INT: dst->payload.tag_int = src->payload.tag_int; break;
			case MT3_TAG_LONG: dst->payload.tag_long = src->payload.tag_long; break;
			case MT3_TAG_FLOAT: dst->payload.tag_float = src->payload.tag_float; break;
			case MT3_TAG_DOUBLE: dst->payload.tag_double = src->payload.tag_double; break;
			case MT3_TAG_STRING:
			{
				MT3_CHECKED_CALLOC(dst->payload.tag_string, dst->length, sizeof(SPchar), return SP_FALSE);
				memcpy(dst->payload.tag_string, src->payload.tag_string, src->length);
				break;
			}
			default:
			{
				dst->payload.tag_object = mt3_Copy(src->payload.tag_object);
				break;
			}
		}
	}
	return SP_TRUE;
}

static SPbool _mt3_copy_payload_from_memory(MT3_node node, MT3_tag tag, SPsize length, const void* value)
{
	if(node)
	{
		switch(tag)
		{
			case MT3_TAG_NULL: break;
			case MT3_TAG_BYTE: node->payload.tag_byte = *(const SPbyte*) value; break;
			case MT3_TAG_SHORT: node->payload.tag_short = *(const SPshort*) value; break;
			case MT3_TAG_INT: node->payload.tag_int = *(const SPint*) value; break;
			case MT3_TAG_LONG: node->payload.tag_long = *(const SPlong*) value; break;
			case MT3_TAG_FLOAT: node->payload.tag_float = *(const SPfloat*) value; break;
			case MT3_TAG_DOUBLE: node->payload.tag_double = *(const SPdouble*) value; break;
			case MT3_TAG_STRING:
			{
				MT3_CHECKED_CALLOC(node->payload.tag_string, length, sizeof(SPchar), return SP_FALSE);
				memcpy(node->payload.tag_string, (const SPchar*) value, length);
				break;
			}
			
			default:
			{
				node->payload.tag_object = mt3_Copy((const MT3_node) value);
				break;
			}
		}
	}
	return SP_TRUE;
}

static MT3_node _mt3_copy_tree(const MT3_node n)
{
    MT3_node tree = NULL;
    if(n)
    {
        tree = mt3_AllocTree();
        if(!tree)
        {
            errno = MT3_STATUS_NO_MEMORY;
            return NULL;
        }

        tree->red = n->red;
        tree->weight = n->weight;
        tree->tag = n->tag;
        tree->length = n->length;


		if(!_mt3_copy_payload_from_node(n, tree))
		{
			errno = MT3_STATUS_NO_MEMORY;
			free(tree);
			return NULL;
		}
		
        tree->major = _mt3_copy_tree(n->major);
        tree->minor = _mt3_copy_tree(n->minor);

        if(tree->major)
           tree->major->parent = tree;

        if(tree->minor)
           tree->minor->parent = tree;
    }
    return tree;	
}


static MT3_node _mt3_copy_list(const MT3_node n)
{
	MT3_node list = NULL;
	if(n)
	{
		SP_ASSERT(mt3_IsTree(n), "Expected tree node");
		if(n->red && !n->parent)
		{
			list = mt3_AllocList();
			MT3_node dst_cursor = list;
			MT3_node src_cursor = NULL;
			
			MT3_FOR_EACH(n, src_cursor)
			{
				SP_ASSERT(src_cursor->red, "Expected list node to be red-coded");
				dst_cursor->tag = src_cursor->tag;
				dst_cursor->length = src_cursor->length;
				
				if(!_mt3_copy_payload_from_node(dst_cursor, src_cursor))
				{
					mt3_Delete(&list);
					return NULL;
				}
				
				dst_cursor->red = SP_TRUE;
				dst_cursor->major = mt3_AllocList();
				MT3_node minor = dst_cursor;
				dst_cursor = dst_cursor->major;
				dst_cursor->minor = minor;
			}
			
			dst_cursor->minor->major = NULL;
			free(dst_cursor);
		}
	}
	return list;
}

MT3_node mt3_Copy(const MT3_node n)
{
	MT3_node ret = NULL;
	if(n)
	{
		if(!(mt3_IsTree(n) || mt3_IsList(n)))
		{
			errno = MT3_STATUS_BAD_VALUE;
			return NULL;
		}
		
		ret = mt3_IsTree(n) ? _mt3_copy_tree(n) : _mt3_copy_list(n);
	}
	return ret;
}

// very dangerous, can cause disruption!!
static void _mt3_delete_payload(MT3_node n)
{
	if(n)
	{		
		switch(n->tag)
		{
			case MT3_TAG_NULL:
				break;
			case MT3_TAG_STRING:
			{
				free(n->payload.tag_string);
				break;
			}
			
			case MT3_TAG_ROOT:
			case MT3_TAG_LIST:
			case MT3_TAG_ROOT_LIST:
			case MT3_TAG_BYTE_LIST:
			case MT3_TAG_SHORT_LIST:
			case MT3_TAG_INT_LIST:
			case MT3_TAG_LONG_LIST:
			case MT3_TAG_FLOAT_LIST:
			case MT3_TAG_DOUBLE_LIST:
			case MT3_TAG_STRING_LIST:
			{
				mt3_Delete(&n->payload.tag_object);
				break;
			}
		}
		free(n);
	}
}
static void _mt3_delete_tree_impl(MT3_node tree)
{
	if(tree)
	{	
		MT3_node major = tree->major;
		MT3_node minor = tree->minor;
		_mt3_delete_payload(tree);
		_mt3_delete_tree_impl(tree->major);
		_mt3_delete_tree_impl(tree->minor);
	}
}

static void _mt3_delete_list_impl(MT3_node list)
{
	if(list)
	{
		MT3_node cursor = NULL;
		for(cursor = list; cursor != NULL;)
		{
			MT3_node major = cursor->major;
			_mt3_delete_payload(cursor);
			cursor = cursor->major;
		}
	}
}

void mt3_Delete(MT3_node* node)
{
	if(node)
	{
		if(!(mt3_IsTree(*node) || mt3_IsList(*node)))
		{
			errno = MT3_STATUS_BAD_VALUE;
			return;
		}
		
		if(mt3_IsTree(*node))
		{
			_mt3_delete_tree_impl(*node);
		}
		else
		{
			_mt3_delete_list_impl(*node);
		}
		*node = NULL;
	}
}

static MT3_node _mt3_search_for(MT3_node tree, SPhash hash)
{
	MT3_node node = NULL;
	if(tree)
	{
		if(tree->weight == hash)
			return tree;
		
		if(hash > tree->weight)
			node = _mt3_search_for(tree->major, hash);
		else
			node = _mt3_search_for(tree->minor, hash);
	}
	return node;
}

MT3_node _mt3_search(const MT3_node tree, const char* name)
{
	MT3_node ret = NULL;
	if(mt3_IsTree(tree))
	{
		SPhash hash = _mt3_sdbm(name);
		ret = _mt3_search_for(tree, hash);
	}
	return ret;
}

static MT3_node _mt3_insert_data(MT3_node* head, MT3_node node, SPhash weight, MT3_tag tag, SPsize length, const void* value, SPbool copyValue)
{
	if(!node)
	{
		SP_DEBUG("from null");
		*head = _mt3_alloc_node(tag, weight, length, value, copyValue);
		return *head;
	}

	if(weight == node->weight)
	{
		errno = MT3_STATUS_BAD_NAME;
		return NULL;
	}

	if(node->weight == 0 || node->tag == MT3_TAG_NULL)
	{
		
		SP_ASSERT(node->length == 0LL, "Empty tree cannot have length defined (%lld)", node->length);
		SP_ASSERT(!node->major && !node->minor, "Empty tree cannot have sub-trees");
		SP_ASSERT(!node->parent, "Empty tree cannot have a parent");
		SP_ASSERT(!node->red, "Empty tree must be black-coded");
		
		_mt3_copy_payload_from_memory(node, tag, length, value);
		node->tag = tag;
		node->weight = weight;
		return *head;
	}

	SPbool maj = (weight > node->weight);
	MT3_node primary  = maj ? node->major : node->minor;

	if(primary)
	{
		_mt3_insert_data(head, primary, weight, tag, length, value, copyValue);
	}
	else
	{	
		primary = _mt3_alloc_node(tag, weight, length, value, copyValue);
		primary->parent = node;
		primary->red = SP_TRUE;
		maj ? (node->major = primary) : (node->minor = primary);
		_mt3_fix_rbt_violations(primary, head);
	}
	return primary;
}

const char* _mt3_tag_to_str(MT3_tag tag)
{
	if(tag & MT3_TAG_LIST)
	{
		MT3_tag scalar = tag & ~MT3_TAG_LIST;
		switch(scalar)
		{
			case MT3_TAG_NULL: return "multi-list";
			case MT3_TAG_BYTE: return "byte-list";
			case MT3_TAG_SHORT: return "short-list";
			case MT3_TAG_INT: return "int-list";
			case MT3_TAG_LONG: return "long-list";
			case MT3_TAG_FLOAT: return "float-list";
			case MT3_TAG_DOUBLE: return "double-list";
			case MT3_TAG_STRING: return "string-list";
			case MT3_TAG_ROOT: return "tree-list";
		}
	}
	else
	{
		switch(tag)
		{
			case MT3_TAG_BYTE: return "byte";
			case MT3_TAG_SHORT: return "short";
			case MT3_TAG_INT: return "int";
			case MT3_TAG_LONG: return "long";
			case MT3_TAG_FLOAT: return "float";
			case MT3_TAG_DOUBLE: return "double";
			case MT3_TAG_STRING: return "string";
			case MT3_TAG_LIST: return "list";
			case MT3_TAG_ROOT: return "tree";
		}
	}
	return "NULL";
}

static void _mt3_print(const MT3_node tree, int level, SPbool printTreeData)
{
	if(tree)
	{
		SPchar color = tree->red ? 'R' : 'B';
		SPchar rank = _mt3_is_root(tree) ? '~' : (_mt3_is_major(tree) ? '+' : '-');
		const char* listFromatStr = printTreeData ? "(%c%c) %s (%lld elements) (%lld): " : "%s (%lld elements): ";
		
		for(int i = 0; i < level; i++)
			printf("\t");
		switch(tree->tag)
		{
			case MT3_TAG_NULL:
			{
				break;
			}
			
			case MT3_TAG_ROOT:
			{
				printf("(%c%c) %s (%lld):\n", color, rank, _mt3_tag_to_str(tree->tag), tree->weight);
				_mt3_print_tree(tree->payload.tag_object, level + 1);
				break;
			}
			
			case MT3_TAG_BYTE:
			{
				printf("(%c%c) %s (%lld): %hhd\n", color, rank, _mt3_tag_to_str(tree->tag), tree->weight, tree->payload.tag_byte);
				break;
			}
			
			case MT3_TAG_SHORT:
			{
				printf("(%c%c) %s (%lld): %hd\n", color, rank, _mt3_tag_to_str(tree->tag), tree->weight, tree->payload.tag_short);
				break;
			}
			
			case MT3_TAG_INT:
			{
				printf("(%c%c) %s (%lld): %d\n", color, rank, _mt3_tag_to_str(tree->tag), tree->weight, tree->payload.tag_int);
				break;
			}
			
			case MT3_TAG_LONG:
			{
				printf("(%c%c) %s (%lld): %lld\n", color, rank, _mt3_tag_to_str(tree->tag), tree->weight, tree->payload.tag_long);
				break;
			}
			
			case MT3_TAG_FLOAT:
			{
				printf("(%c%c) %s (%lld): %f\n", color, rank, _mt3_tag_to_str(tree->tag), tree->weight, tree->payload.tag_float);
				break;
			}
			
			case MT3_TAG_DOUBLE:
			{
				printf("(%c%c) %s (%lld): %f\n", color, rank, _mt3_tag_to_str(tree->tag), tree->weight, tree->payload.tag_double);
				break;
			}
			
			case MT3_TAG_STRING:
			{
				printf("(%c%c) %s (%lld): \"%s\"\n", color, rank, _mt3_tag_to_str(tree->tag), tree->weight, tree->payload.tag_string);
				break;
			}
			
			
			default:
			{
				SP_ASSERT(tree->length == _mt3_length_of_list(tree->payload.tag_object), "Expected equal length for list");
				printf("(%c%c) %s (%lld elements) (%lld):\n", color, rank, _mt3_tag_to_str(tree->tag), tree->length, tree->weight);
				_mt3_print_list(tree->payload.tag_object, level + 1);
				break;
			}
		}
	}
}

void _mt3_print_list(const MT3_node list, int level)
{
	if(list)
	{
		
		MT3_node cursor = NULL;
		MT3_FOR_EACH(list, cursor)
		{
			switch(cursor->tag)
			{
				case MT3_TAG_NULL:
					return;
				case MT3_TAG_ROOT:
				{
					_mt3_print_tree(cursor->payload.tag_object, level);
					break;
				}
				
				case MT3_TAG_LIST:
				case MT3_TAG_ROOT_LIST:
				case MT3_TAG_BYTE_LIST:
				case MT3_TAG_SHORT_LIST:
				case MT3_TAG_INT_LIST:
				case MT3_TAG_LONG_LIST:
				case MT3_TAG_FLOAT_LIST:
				case MT3_TAG_DOUBLE_LIST:
				case MT3_TAG_STRING_LIST:
				{
					for(int i = 0; i < level; i++)
						printf("\t");
					
					printf("%s (%lld elements):\n", _mt3_tag_to_str(cursor->tag), _mt3_length_of_list(cursor->payload.tag_object));
					_mt3_print_list(cursor->payload.tag_object, level + 1);
					break;
				}
				
				default:
				{
					_mt3_print(cursor, level, SP_FALSE);
					break;
				}
			}
		}
	}
}

void _mt3_print_tree(const MT3_node tree, int level)
{
	if(tree)
	{
		_mt3_print(tree, level, SP_TRUE);
		_mt3_print_tree(tree->major, level + 1);
		_mt3_print_tree(tree->minor, level + 1);
	}
}

void mt3_PrintTree(const MT3_node tree)
{
	if(tree && mt3_IsTree(tree))
	{
		_mt3_print_tree(tree, 0);
		printf("\n~ ... Root\n");
		printf("+ ... Major\n");
		printf("- ... Minor\n");
		printf("B ... Black\n");
		printf("R ... Red\n\n");
		printf("\n");
	}
}

void mt3_InsertByte(MT3_node* tree, const SPchar* name, SPbyte value)
{
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_BYTE, sizeof(SPbyte), &value, SP_TRUE);
}

void mt3_InsertShort(MT3_node* tree, const SPchar* name, SPshort value)
{
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_SHORT, sizeof(SPshort), &value, SP_TRUE);
}

void mt3_InsertInt(MT3_node* tree, const SPchar* name, SPint value)
{
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_INT, sizeof(SPint), &value, SP_TRUE);
}

void mt3_InsertLong(MT3_node* tree, const SPchar* name, SPlong value)
{
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_LONG, sizeof(SPlong), &value, SP_TRUE);
}

void mt3_InsertFloat(MT3_node* tree, const SPchar* name, SPfloat value)
{
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_FLOAT, sizeof(SPfloat), &value, SP_TRUE);
}

void mt3_InsertDouble(MT3_node* tree, const SPchar* name, SPdouble value)
{
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_DOUBLE, sizeof(SPdouble), &value, SP_TRUE);
}

void mt3_InsertString(MT3_node* tree, const SPchar* name, const SPchar* value)
{
	if(!value)
	{
		errno = MT3_STATUS_BAD_VALUE;
		return;
	}
	
	MT3_CHECK_INPUT(name);
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_STRING, strlen(value) + 1, value, SP_TRUE);
}

void mt3_InsertByteList(MT3_node* tree, const char* name, SPsize length, const SPbyte* values)
{
	MT3_CHECK_INPUT(name);
	if(!values || length <= 0)
	{
		errno = MT3_STATUS_BAD_VALUE;
		return;
	}
	
	MT3_node list = mt3_ToList(MT3_TAG_BYTE, length, values);
	SP_ASSERT(list, "Expected non-NULL list");
	_mt3_insert_data(tree, *tree, hash, MT3_TAG_BYTE_LIST, length, list, SP_FALSE);
}

void mt3_InsertObject(MT3_node* tree, const char* name, SPsize length, const SPbyte* values)
{
	
}

SPsize _mt3_length_of_list(const MT3_node list)
{
	SPsize length = 0;
	if(list)
	{
		for(MT3_node cursor = list; cursor != NULL; cursor = cursor->major)
			length++;
	}
	return length;
}

MT3_node mt3_ToList(MT3_tag tag, SPsize length, const void* data)
{
	MT3_node list = NULL;
	for(SPsize i = 0; i < length; i++)
	{
		switch(tag)
		{
			case MT3_TAG_BYTE: mt3_AppendByte(&list, *(((const SPbyte*) data) + i)); break;
		}
	}
	return list;
}

void _mt3_insert_multi_list(MT3_node* _list, MT3_tag tag, SPsize length, const void* values)
{
	if(_list)
	{
		MT3_node list = *_list;
		if(!list)
		{
			*_list = mt3_AllocList();
			(*_list)->tag = tag;
			_mt3_copy_payload_from_memory(*_list, tag, length, values);
		}
		else
		{
			if(!(list->red && !list->parent))
			{
				errno = MT3_STATUS_BAD_VALUE;
				return;
			}
			
			if(list->tag == MT3_TAG_NULL)
			{
				_mt3_copy_payload_from_memory(list, tag, length, values);
				list->tag = tag;
			}
			else
			{
				if(list->tag == tag)
				{
					MT3_node cursor = list;
					while(cursor->major)
						cursor = cursor->major;
					
					cursor->major = mt3_AllocList();
					cursor->major->tag = tag;
					cursor->major->minor = cursor;
					_mt3_copy_payload_from_memory(cursor->major, tag, length, values);
				}
			}
		}
	}
}

void mt3_AppendByte(MT3_node* list, SPbyte value)
{
	_mt3_insert_multi_list(list, MT3_TAG_BYTE, sizeof(SPbyte), &value);
}

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic pop
#endif