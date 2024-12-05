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
#include "internal.h"
#include <zlib.h>
#include <errno.h>

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wformat"
#endif

#define MT3_READ_GENERIC(dst, n, scanner, fail)     		\
	do							\
	{							\
		if(*length < (n))				\
		{						\
			errno = MT3_STATUS_READ_ERROR;		\
			fail;					\
		}						\
		*memory = scanner((dst), *memory, (n));		\
		*length -= n;					\
	} while(0)
		
#define ne2be _mt3_big_endian_to_native_endian
#define be2ne _mt3_big_endian_to_native_endian

static SPbool _mt3_is_little_endian()
{
	SPuint16 t = 0x0001;
	SPchar c[2];
	memcpy(c, &t, sizeof t);
	return c[0];
}

static void* _mt3_swap_bytes(void* s, SPsize length)
{
	for(SPchar *b = s, *e = b + length - 1; b < e; b++, e--)
	{
		SPchar t = *b;
		*b = *e;
		*e = t;
	}
	return s;
}

static void* _mt3_big_endian_to_native_endian(void* s, size_t len)
{
	return _mt3_is_little_endian() ? _mt3_swap_bytes(s, len) : s;
}

static const void* _mt3_memcpy(void* dst, const void* src, SPsize n)
{
	memcpy(dst, src, n);
	return (const SPchar*) src + n;
}

static const void* _mt3_swapped_memcpy(void* dst, const void* src, SPsize n)
{
	const void* ret = _mt3_memcpy(dst, src, n);
	return ne2be(dst, n), ret;
}

SPbuffer _mt3_compress(const void* memory, SPsize length)
{
	const SPsize CHUNK_SIZE = 4096;
	SPbuffer buffer = SP_BUFFER_INIT;

	z_stream stream =
	{
		.zalloc   = Z_NULL,
		.zfree	  = Z_NULL,
		.opaque   = Z_NULL,
		.next_in  = (void*) memory,
		.avail_in = length
	};

	if(deflateInit2(
		&stream,
		Z_DEFAULT_COMPRESSION,
		Z_DEFLATED,
		15,
		8,
		Z_DEFAULT_STRATEGY) != Z_OK)
	{
	    errno = MT3_STATUS_WRITE_ERROR;
		return SP_BUFFER_INIT;
	}

    	if(stream.avail_in != length)
    	{
        	errno = MT3_STATUS_WRITE_ERROR;
        	return buffer;
    	}

	do
	{
		if(!spBufferReserve(&buffer, buffer.length + CHUNK_SIZE))
		{
			errno = MT3_STATUS_NO_MEMORY;
			spBufferFree(&buffer);
			return SP_BUFFER_INIT;
		}

		stream.next_out = buffer.data + buffer.length;
		stream.avail_out = CHUNK_SIZE;

		if(deflate(&stream, Z_FINISH) == Z_STREAM_ERROR)
		{
			errno = MT3_STATUS_WRITE_ERROR;
			spBufferFree(&buffer);
			return SP_BUFFER_INIT;
		}

		buffer.length += CHUNK_SIZE - stream.avail_out;
	} while(!stream.avail_out);

	deflateEnd(&stream);
	return buffer;
}

SPbuffer _mt3_decompress(const void* memory, SPsize length)
{
	const SPsize CHUNK_SIZE = 4096;
	SPbuffer buffer = SP_BUFFER_INIT;

	z_stream stream =
	{
		.zalloc   = Z_NULL,
		.zfree	  = Z_NULL,
		.opaque   = Z_NULL,
		.next_in  = (void*) memory,
		.avail_in = length
	};

	if(inflateInit2(&stream, 47) != Z_OK)
	{
	    errno = MT3_STATUS_READ_ERROR;
		return SP_BUFFER_INIT;
	}

	SPint zlib_ret;
	do
	{
		if(!spBufferReserve(&buffer, buffer.length + CHUNK_SIZE))
		{
		    errno = MT3_STATUS_NO_MEMORY;
			spBufferFree(&buffer);
			return SP_BUFFER_INIT;
		}

		stream.avail_out = CHUNK_SIZE;
		stream.next_out = (SPubyte*) buffer.data + buffer.length;

		switch((zlib_ret = inflate(&stream, Z_NO_FLUSH)))
		{
			case Z_MEM_ERROR:
			case Z_DATA_ERROR:
			case Z_NEED_DICT:
			{
				errno = MT3_STATUS_READ_ERROR;
				spBufferFree(&buffer);
				return SP_BUFFER_INIT;
			}
			default:
				buffer.length += CHUNK_SIZE - stream.avail_out;
		}
	} while(!stream.avail_out);

	if(zlib_ret != Z_STREAM_END)
	{
		errno = MT3_STATUS_READ_ERROR;
		return SP_BUFFER_INIT;
	}

	inflateEnd(&stream);
	return buffer;
}

static void* _mt3_alloc_chunk(SPsize size)
{
	SPbyte* ptr = NULL;
	MT3_CHECKED_CALLOC(ptr, size, sizeof(SPbyte), return NULL);
	return ptr;
}

static void _mt3_write_bytes(SPbuffer* buffer, const SPubyte* src, SPsize length, SPbool toNativeEndian, int level)
{
	SPubyte* chunk = _mt3_alloc_chunk(length);
	if(toNativeEndian)
		_mt3_swapped_memcpy(chunk, src, length);
	else
		_mt3_memcpy(chunk, src, length);
	
	if(!spBufferAppend(buffer, chunk, length))
	{
		free(chunk);
		errno = MT3_STATUS_WRITE_ERROR;
		return;
	}
	free(chunk);
}

static void _mt3_encode(const MT3_node node, SPbuffer* buffer, int level)
{	
	if(node)
	{
		switch(node->tag)
		{
			case MT3_TAG_BYTE: _mt3_write_bytes(buffer, (const SPubyte*) &node->payload.tag_byte, sizeof(SPbyte), SP_TRUE, level); break;
			case MT3_TAG_SHORT: _mt3_write_bytes(buffer, (const SPubyte*) &node->payload.tag_short, sizeof(SPshort), SP_TRUE, level); break;
			case MT3_TAG_INT: _mt3_write_bytes(buffer, (const SPubyte*) &node->payload.tag_int, sizeof(SPint), SP_TRUE, level); break;
			case MT3_TAG_LONG: _mt3_write_bytes(buffer, (const SPubyte*) &node->payload.tag_long, sizeof(SPlong), SP_TRUE, level); break;
			case MT3_TAG_FLOAT: _mt3_write_bytes(buffer, (const SPubyte*) &node->payload.tag_float, sizeof(SPfloat), SP_TRUE, level); break;
			case MT3_TAG_DOUBLE: _mt3_write_bytes(buffer, (const SPubyte*) &node->payload.tag_double, sizeof(SPdouble), SP_TRUE, level); break;
			case MT3_TAG_STRING:
			{
				SP_ASSERT(node->payload.tag_string, "Node %lld has invalid data to write", node->weight);

				_mt3_write_bytes(buffer, (const SPubyte*) &node->length, sizeof(SPsize), SP_TRUE, level);
				_mt3_write_bytes(buffer, (const SPubyte*) node->payload.tag_string, node->length, SP_FALSE, level);
				break;
			}
			
			case MT3_TAG_ROOT:
			{
				_mt3_encode_tree(node->payload.tag_object, buffer, level + 1);
				break;
			}
			
			default:
			{
				_mt3_write_bytes(buffer, (const SPubyte*) &node->length, sizeof(SPsize), SP_TRUE, level);
				_mt3_encode_list(node->payload.tag_object, buffer, level);
				break;
			}
		}
	}
}

void _mt3_encode_tree(const MT3_node tree, SPbuffer* buffer, int level)
{
	if(!tree)
	{
		MT3_tag null = MT3_TAG_NULL;
		_mt3_write_bytes(buffer, (const SPubyte*) &null, sizeof(SPbyte), SP_FALSE, level);
		return;
	}
	
	//encode the tag
	SPuint8 tag = tree->tag | ((tree->red & 1) << 6);
	_mt3_write_bytes(buffer, (const SPubyte*) &tag, sizeof(SPbyte), SP_FALSE, level);

	//encode the weight
	_mt3_write_bytes(buffer, (const SPubyte*) &tree->weight, sizeof(SPlong), SP_TRUE, level);

	//encode generic
	_mt3_encode(tree, buffer, level);

	_mt3_encode_tree(tree->major, buffer, level + 1);
	_mt3_encode_tree(tree->minor, buffer, level + 1);
}

void _mt3_encode_list(const MT3_node list, SPbuffer* buffer, int level)
{
    if(list)
    {
        SP_ASSERT(mt3_IsList(list), "Expected list to encode");
        MT3_node cursor = NULL;
        SPsize i = 0;

        for(cursor = list; cursor != NULL; cursor = cursor->major)
        {
            if(cursor->tag & MT3_TAG_LIST)
            {
                //encode the tag, if it is a multi-list
                SPuint8 tag = list->tag | ((list->red & 1) << 6);
                _mt3_write_bytes(buffer, (const SPubyte*) &tag, sizeof(SPbyte), SP_FALSE, level);
            }
            _mt3_encode(cursor, buffer, level);
        }
	}
}

SPbuffer mt3_EncodeTree(const MT3_node tree)
{
	SPbuffer buffer = SP_BUFFER_INIT;
	
	if(!mt3_IsTree(tree))
	{
		errno = MT3_STATUS_NOT_A_TREE;
		return buffer;
	}
	
	_mt3_encode_tree(tree, &buffer, 0);
	SPbuffer compressed = _mt3_compress(buffer.data, buffer.length);
	spBufferFree(&buffer);
	return compressed;
}

static SPbool _mt3_decode(MT3_node node, const SPubyte** memory, SPsize* length)
{
	if(node)
	{
		SP_ASSERT(node->tag != MT3_TAG_NULL, "Expected type");
		switch(node->tag)
		{
			case MT3_TAG_BYTE: MT3_READ_GENERIC(&node->payload.tag_byte, sizeof(SPbyte), _mt3_swapped_memcpy, return SP_FALSE); node->length = sizeof(SPbyte); break;
			case MT3_TAG_SHORT: MT3_READ_GENERIC(&node->payload.tag_short, sizeof(SPshort), _mt3_swapped_memcpy, return SP_FALSE); node->length = sizeof(SPshort); break;
			case MT3_TAG_INT: MT3_READ_GENERIC(&node->payload.tag_int, sizeof(SPint), _mt3_swapped_memcpy, return SP_FALSE); node->length = sizeof(SPint); break;
			case MT3_TAG_LONG: MT3_READ_GENERIC(&node->payload.tag_long, sizeof(SPlong), _mt3_swapped_memcpy, return SP_FALSE); node->length = sizeof(SPlong); break;
			case MT3_TAG_FLOAT: MT3_READ_GENERIC(&node->payload.tag_float, sizeof(SPfloat), _mt3_swapped_memcpy, return SP_FALSE); node->length = sizeof(SPfloat); break;
			case MT3_TAG_DOUBLE: MT3_READ_GENERIC(&node->payload.tag_double, sizeof(SPdouble), _mt3_swapped_memcpy, return SP_FALSE); node->length = sizeof(SPdouble); break;
			case MT3_TAG_STRING:
			{
				MT3_READ_GENERIC(&node->length, sizeof(SPlong), _mt3_swapped_memcpy, return SP_FALSE);
				if(node->length)
				{
					MT3_CHECKED_CALLOC(node->payload.tag_string, node->length, sizeof(SPbyte), return SP_FALSE);
					MT3_READ_GENERIC(node->payload.tag_string, node->length, _mt3_memcpy, return SP_FALSE);
				}
				break;
			}
			case MT3_TAG_ROOT:
			{
				node->payload.tag_object = _mt3_decode_tree(memory, length);
				break;
			}
			
			default:
			{
                _mt3_decode_list(node, memory, length);
				break;
			}
		}
	}
	return SP_TRUE;
}

MT3_node _mt3_decode_tree(const SPubyte** memory, SPsize* length)
{
	SPubyte tag;
	MT3_READ_GENERIC(&tag, sizeof(SPubyte), _mt3_memcpy, return NULL);
	
	SPbool redness = (tag >> 6) & 1;
	tag &= ~0x40;
	
	if(tag == MT3_TAG_NULL)
		return NULL;

	MT3_node tree;
	MT3_CHECKED_CALLOC(tree, 1, sizeof(struct _MT3_node), return NULL);
	MT3_READ_GENERIC(&tree->weight, sizeof(SPlong), _mt3_swapped_memcpy, return NULL);

	tree->red = redness;
	tree->tag = tag;
	
	if(!_mt3_decode(tree, memory, length))
		return NULL;

	tree->major = _mt3_decode_tree(memory, length);
	tree->minor = _mt3_decode_tree(memory, length);

	if(tree->major)
	    tree->major->parent = tree;
	if(tree->minor)
		tree->minor->parent = tree;

	return tree;
}


SPbool _mt3_decode_list(MT3_node node, const SPubyte** memory, SPsize* length)
{
    if(!node)
        return SP_FALSE;

    MT3_READ_GENERIC(&node->length, sizeof(SPlong), _mt3_swapped_memcpy, return SP_FALSE);
    if(node->length)
    {
        MT3_node array = mt3_AllocList();
        MT3_node cursor = array;

        for(SPsize i = 0; i < node->length; i++)
        {
            SPubyte tag;
            SPbool redness = SP_FALSE;
            if(node->tag == MT3_TAG_LIST)
            {
                MT3_READ_GENERIC(&tag, sizeof(SPbyte), _mt3_memcpy, return SP_FALSE);
                redness = (tag >> 6) & 1;
                if(!redness)
                {
                    mt3_Delete(&array);
                    return SP_FALSE;
                }
                tag &= ~0x40;
            }
            else
            {
                tag = node->tag & ~MT3_TAG_LIST;
                redness = SP_TRUE;
            }

            cursor->tag = tag;
            cursor->red = redness;

            if(!_mt3_decode(cursor, memory, length))
            {
                mt3_Delete(&array);
                return SP_FALSE;
            }
            
            cursor->major = mt3_AllocList();
            MT3_node minor = cursor;
            cursor = cursor->major;
            cursor->minor = minor;
        }

        if(cursor->minor)
           cursor->minor->major = NULL;

        free(cursor);
        node->payload.tag_object = array;
    }
    return SP_TRUE;
}

MT3_node mt3_DecodeTree(SPbuffer buffer)
{
	SPbuffer decompressed = _mt3_decompress(buffer.data, buffer.length);
	const SPubyte* memory = decompressed.data;
	MT3_node ret = _mt3_decode_tree((const SPubyte**) &memory, &decompressed.length);
	spBufferFree(&decompressed);
	return ret;
}

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic pop
#endif
