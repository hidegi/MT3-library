#ifndef SIMPLE_RESOURCE_LOADER_H
#define SIMPLE_RESOURCE_LOADER_H
#include "SP/config.h"

namespace sp
{
	SP_API SPubyte* loadImage(const char* path,
					int& width,
					int& height,
					int& channels,
					int requested);
	SP_API SPubyte* loadData(const char* path);
	SP_API SPubyte* loadData(const char* path, size_t& fileSize);
	SP_API SPbool writeData(const char* output, const SPubyte* data, SPsize count);
	SP_API void freeResource(const void* res);
}

#endif // SIMPLE_RESOURCE_LOADER_H
