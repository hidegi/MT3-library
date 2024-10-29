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
	
	return unlikely(!b->data) ? MOT_ERR_GEN : MOT_ERR_NONE;
}

void spBufferFree(SPbuffer* b)
{
	SP_ASSERT(b, "Cannot free uninitialized buffer");
	free(b->data);
	b->data = NULL;
	b->length = b->capacity = 0;
}

int spBufferReserve(SPbuffer* b, SPsize reserved)
{
	SP_ASSERT(b, "Cannot reserve for uninitialized buffer");
	if(unlikely(!b->data) && unlikely(_spLazyInit(b)))
	{
		SP_WARNING("Failed to initialize buffer");
		return MOT_ERR_GEN;
	}
	if(likely(b->capacity >= reserved))
		return MOT_ERR_NONE;
	
	while(b->capacity < reserved)
		b->capacity *= 2;
	
	unsigned char* tmp = realloc(b->data, b->capacity);
	if(unlikely(!tmp))
	{
		SP_WARNING("Failed to resize buffer for size %lld", b->capacity);
		spBufferFree(b);
		return MOT_ERR_GEN;
	}
	
	b->data = tmp;
	return MOT_ERR_NONE;
}

int spBufferAppend(SPbuffer* b, const void* data, SPsize n)
{
	SP_ASSERT(b, "Cannot append to empty buffer");
	if(unlikely(!b->data) && unlikely(_spLazyInit(b)))
	{
		SP_WARNING("Failed to initialize buffer");
		return MOT_ERR_GEN;
	}
	
	if(unlikely(spBufferReserve(b, b->length + n)))
	{
		SP_WARNING("Failed to reserve data");
		return MOT_ERR_GEN;
	}
	
	memcpy(b->data + b->length, data, n);
	b->length += n;
	return MOT_ERR_NONE;
} 