#include "SP/utils/io.h"
#include "SP/cnc/thread.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <iostream>
#include <fstream>
namespace
{
	sp::Mutex resourceGuard;
}
namespace sp
{
	//file loaders must be performed with exclusive access..
	SPubyte* loadImage(const char* path, int& w, int& h, int& c, int requested)
	{
		sp::Lock lock(resourceGuard);
		int width;
		int height;
		int channels;
		width = height = channels = 0;
		SPubyte* buffer = reinterpret_cast<SPubyte*>(stbi_load(path, &width, &height, &channels, requested));
		if(buffer)
		{
			w = width;
			h = height;
			c = requested;
		}
		return buffer;
	}

	SPubyte* loadData(const char* path)
	{
		size_t dummy;
		return loadData(path, dummy);
	}
	
	SPubyte* loadData(const char* path, size_t& fileSize)
	{
		sp::Lock lock(resourceGuard);
		FILE* file = NULL;
		const char* mode = "rb";
#ifdef SP_PLATFORM_WINDOWS
	wchar_t wmode[64];
	wchar_t wpath[1024];
	if(!MultiByteToWideChar(65001, 0, path, -1, wpath, sizeof(wpath) / sizeof(*wpath)))
	{
		SP_WARNING("Win32: failed to convert string to wide char for file %s", path);
		return NULL;
	}
	if(!MultiByteToWideChar(65001, 0, mode, -1, wmode, sizeof(wmode) / sizeof(*wmode)))
	{
		SP_WARNING("Win32: failed to convert open mode string to wide char for file %s", path);
		return NULL;
	}
#if defined(SP_MSC_VER) && SP_MSC_VER >= 1400
		if(_wfopen_s(&file, wpath, wmode))
			file = NULL;
#else
		file = _wfopen(wpath, wmode);
#endif // defined
		if(fopen_s(&file, path, mode))
			file = NULL;
#else
		file = fopen(path, mode);
#endif // SP_PLATFORM_WINDOWS
		if(!file)
		{
			SP_WARNING("failed to open file %s", path);
			return NULL;
		}

		
		fseek(file, 0, SEEK_END);
		const long bytes = ftell(file);
		fseek(file, 0, SEEK_SET);

		if((bytes <= 0))
		{
			SP_WARNING("invalid byte size");
			fclose(file);
			return NULL;
		}
		
		fileSize = (size_t) bytes;
		SPubyte* buffer = (SPubyte*) calloc(bytes, sizeof(SPubyte));
		if(fread(buffer, sizeof(SPubyte), bytes, file) != static_cast<size_t>(bytes))
		{
			SP_WARNING("specified byte size does not match read byte size");
			free(buffer);
			buffer = NULL;
		}

		fclose(file);
		return buffer;
	}
	
	SPbool writeData(const char* output, const SPubyte* data, SPsize count)
	{
		sp::Lock lock(resourceGuard);
		std::ofstream out;
		out.open(output, std::ios::out | std::ios::binary | std::ios::trunc);
		SP_ASSERT(out.is_open(), "Failed to open file \"%s\"", output);
		
		out.write(reinterpret_cast<const char*>(data), sizeof(SPchar) * count);
		out.close();
		return SP_TRUE;
	}

	void freeResource(const void* data)
	{
		free(const_cast<void*>(data));
		data = NULL;
	}
}
