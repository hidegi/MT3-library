#include "SP/sparse/mot.h"
#include <errno.h>

#define MOT_CHECKED_CALLOC(ptr, n, size, on_error)	\
	do												\
	{												\
		if(!((ptr) = calloc(n, size)))				\
		{											\
			errno = MOT_ERR_MEM;					\
			on_error;								\
		}											\
	} while(0)
		
#define MOT_SET_ID(dst, src, on_error)           	\
	do												\
	{                                              	\
	    if(!src)                                	\
	    {                                      		\
	        errno = MOT_ERR_GEN;					\
	        on_error;								\
	    }                                          	\
		(dst)->id = sdbm_hash(src);					\
	} while(0)
		
	
long long sdbm_hash(const char *str) {
    long long hash = 0;
    int c;
    while ((c = *str++)) {
        hash = c + (hash << 6) + (hash << 16) - hash; // hash * 65599 + c
    }

    return hash;
}

long long sdbmHashGPT(const char* str)
{
	SPulong hash = sdbm_hash(str);
    SPbyte buffer[4];
    // Split the hash into 8 bytes
    for (int i = 0; i < 8; i++) {
        buffer[i] = (hash >> (i * 8)) & 0xFF; // Get the i-th byte
    }
	
	long long output = 0;
	memcpy(&output, buffer, sizeof(SPbyte) * 4);
	return output;
}

long long sdbmHashGithub(const char* str)
{
	SPsize len = strlen(str);
	register unsigned long n = 0;
	while (len--)
		n = *str++ + 65599 * n;
	return n;
}

MOT_tree* motAllocTree(const SPchar* name)
{
	struct MOT_node* root = NULL;
	MOT_CHECKED_CALLOC(root, 1, sizeof(struct MOT_node), return NULL);
	MOT_SET_ID(root, name, return NULL);
	root->tag = MOT_TAG_TREE;
	root->major = NULL;
	root->minor = NULL;
	return root;
}

void motFreeTree(MOT_tree* tree)
{
	if(tree)
	{
		switch(tree->tag)
		{
			case MOT_TAG_TREE:
			{
				break;
			}
			case MOT_TAG_INTEGER:
			case MOT_TAG_FLOAT:
			case MOT_TAG_STRING:
			{
				free(tree->payload.buffer.data);
				break;
			}
		}
		motFreeTree(tree->major);
		motFreeTree(tree->minor);
		
		SP_DEBUG("freeing %lld", tree->id);
		free(tree);
		/*
		if(tree->tag & MOT_TAG_ARRAY)
		{
			SPubyte tag = tree->tag & 0xF;
			switch(tag)
			{
				case MOT_TAG_TREE:
				{
					for(SPsize i = 0; i < tree->payload.buffer.length; i++)
						motFreeTree((MOT_tree*)tree->payload.buffer.data[i]);
				}
				case MOT_TAG_INTEGER:
				case MOT_TAG_FLOAT:
				case MOT_TAG_STRING:
				{
					free(tree->payload.buffer.data);
					break;
				}
			}
		}
		*/
	}
}

void motAddIntegerImpl(MOT_tree* tree, SPlong name, SPint value)
{
	SP_ASSERT(tree, "Cannot add integer to empty tree");
	SP_ASSERT(name != tree->id, "Name has already been given to a node");
	
	if(name > tree->id)
	{
		if(!tree->major)
		{
			struct MOT_node* node = NULL;
			MOT_CHECKED_CALLOC(node, 1, sizeof(struct MOT_node), return);
			node->tag = MOT_TAG_INTEGER;
			node->id = name;
			node->payload.buffer.length = sizeof(SPint);
			node->payload.buffer.stride = 1;
			node->major = node->minor = NULL;
			MOT_CHECKED_CALLOC(node->payload.buffer.data, node->payload.buffer.length, sizeof(SPbyte), return);
			memcpy(node->payload.buffer.data, &value, node->payload.buffer.length * sizeof(SPbyte));
			tree->major = node;
			return;
		}
		else
		{
			motAddIntegerImpl(tree->major, name, value);
		}
	}
	else
	{
		if(!tree->minor)
		{
			struct MOT_node* node = NULL;
			MOT_CHECKED_CALLOC(node, 1, sizeof(struct MOT_node), return);
			node->tag = MOT_TAG_INTEGER;
			node->id = name;
			node->payload.buffer.length = sizeof(SPint);
			node->payload.buffer.stride = 1;
			node->major = node->minor = NULL;
			MOT_CHECKED_CALLOC(node->payload.buffer.data, node->payload.buffer.length, sizeof(SPbyte), return);
			memcpy(node->payload.buffer.data, &value, node->payload.buffer.length * sizeof(SPbyte));
			tree->minor = node;
			return;
		}
		else
		{
			motAddIntegerImpl(tree->minor, name, value);
		}
	}
}

void motAddInteger(MOT_tree* tree, const SPchar* name, SPint value)
{
	long long hash = sdbm_hash(name);
	motAddIntegerImpl(tree, hash, value);
}

/*<==========================================================>*
 *  debug
 *<==========================================================>*/
 
void motPrintTreeImpl(const MOT_tree* tree, int level)
{
	if(tree)
	{
		for(int i = 0; i < level; i++)
            printf("\t");
		switch(tree->tag)
		{
			case MOT_TAG_TREE:
			{
				printf("Tree (%lld):\n", tree->id);
				break;
			}
			case MOT_TAG_INTEGER:
			{
				SPbyte* buffer = tree->payload.buffer.data;
				SP_ASSERT(buffer, "Node has invalid data");
				SP_ASSERT(tree->payload.buffer.stride == 1L, "Node is supposed to hold one value");
				SP_ASSERT(tree->payload.buffer.length <= 8 && tree->payload.buffer.length > 0, "Invalid data size for an integer");
				
				long long l = 0;
				memcpy(&l, buffer, tree->payload.buffer.length * sizeof(SPubyte));
				printf("Number (%lld): %lld\n", tree->id, l);
				break;
			}
		}
		motPrintTreeImpl(tree->major, level + 1);
		motPrintTreeImpl(tree->minor, level + 1);
	}
}
void motPrintTree(const MOT_tree* tree)
{
	motPrintTreeImpl(tree, 0);
	printf("\n");
}