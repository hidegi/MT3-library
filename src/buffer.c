/*
 * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Lukas Niederbremer <webmaster@flippeh.de> and Clark Gaebel <cg.wowus.cg@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */
#include "mt3.h"
#include "buffer.h"

#ifdef SP_COMPILER_GNUC
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(  (x), 0)
#else
#define likely(x)   (x)
#define unlikely(x) (x)
#endif

static int _spLazyInit(SPbuffer* b)
{
    SP_ASSERT(!b->data, "Buffer already allocated");
    SPsize capacity = 1024;
    b->data = malloc(capacity);
    b->length = 0;
    b->capacity = capacity;
    
    return unlikely(!b->data) ? SP_FALSE : SP_TRUE;
}

void spBufferFree(SPbuffer* b)
{
	if(b->data)
	{
		free(b->data);
		b->data = NULL;
		b->length = b->capacity = 0;
	}
}

SPbool spBufferReserve(SPbuffer* b, SPsize reserved)
{
    SP_ASSERT(b, "Cannot reserve for NULL");
    if(unlikely(!b->data) && unlikely(!_spLazyInit(b)))
    {
        return SP_FALSE;
    }
    if(likely(b->capacity >= reserved))
        return SP_TRUE;
    
    while(b->capacity < reserved)
        b->capacity *= 2;
    
    unsigned char* tmp = realloc(b->data, b->capacity);
    if(unlikely(!tmp))
    {
        spBufferFree(b);
        return SP_FALSE;
    }
    
    b->data = tmp;
    return SP_TRUE;
}

SPbool spBufferAppend(SPbuffer* b, const void* data, SPsize n)
{
    SP_ASSERT(b, "Cannot append to NULL");
    if(unlikely(!b->data) && unlikely(!_spLazyInit(b)))
    {
        return SP_FALSE;
    }
    
    if(unlikely(!spBufferReserve(b, b->length + n)))
    {
        return SP_FALSE;
    }
    
    memcpy(b->data + b->length, data, n);
    b->length += n;
    return SP_TRUE;
} 