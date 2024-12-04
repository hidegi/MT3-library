#ifndef SP_BUFFER_H
#define SP_BUFFER_H
#include "platform.h"
/*
 * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Lukas Niederbremer <webmaster@flippeh.de> and Clark Gaebel <cg.wowus.cg@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */
#include "types.h"
#include <stddef.h>

#define SP_BUFFER_INIT (struct SPbuffer) {NULL, 0, 0}

#ifdef __cplusplus
extern "C" {
#endif
typedef struct SPbuffer
{
	SPubyte* data;
	SPsize length;
	SPsize capacity;
} SPbuffer;

SP_API void spBufferFree(SPbuffer* b);
SP_API SPbool spBufferReserve(SPbuffer* b, SPsize reserved);
SP_API SPbool spBufferAppend(SPbuffer* b, const void* data, SPsize n);

#ifdef __cplusplus
}
#endif
#endif