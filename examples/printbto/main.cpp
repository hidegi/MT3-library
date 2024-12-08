#include "mt3.h"
#include "buffer.h"
#include <getopt.h>
#include <fstream>

static void print(const char* path);
int main(int argc, char** argv)
{
	int c;

    	for (;;)
    	{
        	static struct option long_options[] =
        	{
            		{"version", no_argument, NULL, 'v'},
            		{NULL,      no_argument, NULL, 0}
        	};

        	int option_index = 0;

        	if ((c = getopt_long(argc, argv, "hv", long_options, &option_index)) < 0)
            		break;

        	switch (c)
        	{
        	    	case 0: 
				break;
            		case 'v': 
				SP_INFO("MT3 printbto v1.0.0");
                		return EXIT_SUCCESS;
            		case 'h':
                		SP_INFO("Usage: ./printbto [bto file]");
                		return EXIT_SUCCESS;
            		case '?':
                		SP_INFO("Usage: ./printbto [bto file]");
                		break;
        	}
    	}

    	if(argc != 2)
    	{
        	SP_INFO("Usage: ./printbto [bto file]");
        	return EXIT_SUCCESS;
    	}
	if (optind < argc)
    	{
		print(argv[optind]);
    	}
    	else
    	{
	    	SP_INFO("Usage: ./printbto [bto file]");
    	}

    	return 0;
}

static void print(const char* path)
{
	std::ifstream file(path);
	if(!file.is_open())
	{
		SP_WARNING("Failed to open file \"%s\"", path);
		return;
	}
	
	file.seekg(0, std::ios::end);
	size_t length = file.tellg();
	file.seekg(0, std::ios::beg);
	
	SPbuffer buffer;
	buffer.length = length;
	buffer.data = (SPubyte*) calloc(length, sizeof(SPubyte));
	
	file.read(reinterpret_cast<char*>(buffer.data), length);
	MT3_node node = mt3_DecodeTree(buffer);
	
	if(!node)
	{
		SP_WARNING("Failed to read \"%s\" (not bto-encoded)", path);
		return;
	}
	
	mt3_Print(node);
	mt3_Delete(&node);
	spBufferFree(&buffer);
	file.close();
}
