#ifndef SIMPLE_RESOURCE_LOADER_H
#define SIMPLE_RESOURCE_LOADER_H
#include "SP/config.h"

namespace sp
{
	SP_API 
	SP_API SPubyte* readFile(const char* path);
	SP_API SPubyte* readFile(const char* path, size_t& fileSize);
	SP_API SPsize writeFile(const char* output, const SPubyte* data, SPsize count);
	SP_API void freeResource(const void* res);
}

#endif // SIMPLE_RESOURCE_LOADER_H
