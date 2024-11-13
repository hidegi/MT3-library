#ifndef SP_BUFFER_H
#define SP_BUFFER_H
#include "SP/config.h"
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
SP_API SPint spBufferReserve(SPbuffer* b, SPsize reserved);
SP_API SPint spBufferAppend(SPbuffer* b, const void* data, SPsize n);

#ifdef __cplusplus
}
#endif
#endif