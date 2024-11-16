#include "SP/utils/io.h"
#include "SP/cnc/thread.h"
#include <iostream>
#include <fstream>
namespace
{
	sp::Mutex readGuard;
	sp::Mutex writeGuard;
	sp::Mutex openGuard;
}
namespace sp
{
	//file loaders must be performed with exclusive access..
	SPubyte* readFile(const char* path)
	{
		size_t dummy;
		return readFile(path, dummy);
	}
	
	FILE* openFile(const SPchar* path, const SPchar* mode)
	{
		sp::Lock lock(openGuard);
		FILE* file = NULL;
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
		return file;
	}
	
	SPubyte* readFile(const char* path, size_t& fileSize)
	{
		sp::Lock lock(readGuard);
		FILE* file = openFile(path, "r+b");
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
	
	SPsize writeFile(const char* path, const SPubyte* data, SPsize count)
	{
		sp::Lock lock(writeGuard);
		FILE* file = openFile(path, "w+b");
	   
		// set to start of file..
		fseek(file, 0, SEEK_SET);

		SPsize length = 0LL;
		while(fwrite(&data, count, 1, file))
			length++;
	   
		fclose(file);
		return length;
	}

	void freeResource(const void* data)
	{
		free(const_cast<void*>(data));
	}
}
