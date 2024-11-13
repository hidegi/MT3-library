#include "SP/sparse/mot.h"
#include "SP/sparse/buffer.h"

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
    
    return unlikely(!b->data) ? MOT_STATUS_NO_MEMORY : MOT_STATUS_OK;
}

void spBufferFree(SPbuffer* b)
{
    SP_ASSERT(b, "Cannot free uninitialized buffer");
    free(b->data);
    b->data = NULL;
    b->length = b->capacity = 0;
}

SPint spBufferReserve(SPbuffer* b, SPsize reserved)
{
    SP_ASSERT(b, "Cannot reserve for uninitialized buffer");
    if(unlikely(!b->data) && unlikely(_spLazyInit(b)))
    {
        return MOT_STATUS_NO_MEMORY;
    }
    if(likely(b->capacity >= reserved))
        return MOT_STATUS_OK;
    
    while(b->capacity < reserved)
        b->capacity *= 2;
    
    unsigned char* tmp = realloc(b->data, b->capacity);
    if(unlikely(!tmp))
    {
        spBufferFree(b);
        return MOT_STATUS_NO_MEMORY;
    }
    
    b->data = tmp;
    return MOT_STATUS_OK;
}

SPint spBufferAppend(SPbuffer* b, const void* data, SPsize n)
{
    SP_ASSERT(b, "Cannot append to empty buffer");
    if(unlikely(!b->data) && unlikely(_spLazyInit(b)))
    {
        return MOT_STATUS_NO_MEMORY;
    }
    
    if(unlikely(spBufferReserve(b, b->length + n)))
    {
        return MOT_STATUS_NO_MEMORY;
    }
    
    memcpy(b->data + b->length, data, n);
    b->length += n;
    return MOT_STATUS_OK;
} 