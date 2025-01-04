#include "json.h"
#include "mt3.h"
#include "buffer.h"
#include <getopt.h>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <float.h>

using JSON = nlohmann::json;

static void convert(const char* inputPath, const char* outputPath);
static MT3_node readFromJson(const char* path);

static MT3_tag getTag(JSON node);
static MT3_tag getNumberTag(JSON node);
static MT3_tag getIntegerTag(SPlong node);
static MT3_tag getDecimalTag(SPdouble node);
static MT3_tag getListTag(JSON node);

static void parse(const char* key, JSON node, MT3_node* object);
static void parseList(JSON node, MT3_node* object);

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
                SP_INFO("MT3 json2bto parser v1.0.0");
                return EXIT_SUCCESS;
            case 'h':
                SP_INFO("Usage: ./json2bto [json file] [output file]");
                return EXIT_SUCCESS;
            case '?':
                SP_INFO("Usage: ./json2bto [json file] [output file]");
                break;
        }
    }

    if(argc != 3)
    {
        SP_INFO("Usage: ./json2bto [json file] [output file]");
        return EXIT_SUCCESS;
    }
    if (optind < argc)
    {
        /* Make sure a file was given */
	convert(argv[optind], argv[optind + 1]);
    }
    else
    {
        SP_INFO("Usage: ./json2bto [json file] [output file]");
    }

    return 0;
}

void convert(const char* input, const char* output)
{
    	MT3_node node = readFromJson(input);
	if(!node)
	{
	    SP_WARNING("Failed to read from file \"%s\"", input);
	    return;
	}

	SPbuffer buffer = mt3_EncodeTree(node);
    	std::ofstream file(output, std::ios::binary | std::ios::trunc);
    	file.write(reinterpret_cast<const char*>(buffer.data), buffer.length);
    
	SP_INFO("Written to \"%s\" (%lld B)", output, buffer.length);
	spBufferFree(&buffer);
    	mt3_Delete(&node);
}

MT3_node readFromJson(const char* path)
{
	std::ifstream f(path, std::ios::in);
	if(!f.is_open())
	{
		SP_WARNING("Failed to open file %s", path);
		return NULL;
	}
	
	JSON doc;
	try
	{
	    doc = JSON::parse(f);
	}
	catch(...)
	{
		SP_WARNING("Failed to read from %s", path);
		return NULL;
	}
	
	MT3_node result = NULL;
	for(auto& element : doc.items())
	{
	    parse(element.key().c_str(), element.value(), &result);
	}
	
	SP_ASSERT(result, "Failed to create root-node");
	f.close();
	return result;
}

static void parse(const char* key, JSON node, MT3_node* object)
{
	SP_ASSERT(object, "Cannot parse tree from NULL");
	MT3_tag tag = getTag(node);
	
	SP_ASSERT(tag != MT3_TAG_NULL, "Cannot parse tree from MT3_TAG_NULL");
	switch(tag)
	{
		case MT3_TAG_NULL: break;
		case MT3_TAG_BYTE: mt3_InsertByte(object, key, node.get<SPbyte>()); break;
		case MT3_TAG_SHORT: mt3_InsertShort(object, key, node.get<SPshort>()); break;
		case MT3_TAG_INT: mt3_InsertInt(object, key, node.get<SPint>()); break;
		case MT3_TAG_LONG: mt3_InsertLong(object, key, node.get<SPlong>()); break;
		case MT3_TAG_FLOAT: mt3_InsertFloat(object, key, node.get<SPfloat>()); break;
		case MT3_TAG_DOUBLE: mt3_InsertDouble(object, key, node.get<SPdouble>()); break;
		case MT3_TAG_STRING: mt3_InsertString(object, key, node.get<std::string>().c_str()); break;
		
		case MT3_TAG_ROOT:
		{
			MT3_node subtree = mt3_AllocTree();
			for(auto& element : node.items())
			{
				parse(element.key().c_str(), element.value(), &subtree);
			}
			
			mt3_Insert(object, key, subtree);
			mt3_Delete(&subtree);

			break;
		}
		
		default:
		{
			SP_ASSERT(node.is_array(), "Expected node to be array");
			MT3_node list = mt3_AllocList();
			
			for(SPsize i = 0; i < node.size(); i++)
			{
				parseList(node[i], &list);
			}
			
			mt3_Insert(object, key, list);
			mt3_Delete(&list);
		}
	}
}

static void parseList(JSON node, MT3_node* object)
{
	SP_ASSERT(object, "Cannot parse list from NULL");
	
	MT3_tag tag = getTag(node);
	SP_ASSERT(tag != MT3_TAG_NULL, "Cannot parse list from MT3_TAG_NULL");
	switch(tag)
	{
		case MT3_TAG_BYTE: mt3_AppendByte(object, node.get<SPbyte>()); break;
		case MT3_TAG_SHORT: mt3_AppendShort(object, node.get<SPshort>()); break;
		case MT3_TAG_INT: mt3_AppendInt(object, node.get<SPint>()); break;
		case MT3_TAG_LONG: mt3_AppendLong(object, node.get<SPlong>()); break;
		case MT3_TAG_FLOAT: mt3_AppendFloat(object, node.get<SPfloat>()); break;
		case MT3_TAG_DOUBLE: mt3_AppendDouble(object, node.get<SPfloat>()); break;
		case MT3_TAG_STRING: mt3_AppendString(object, node.get<std::string>().c_str()); break;
		case MT3_TAG_ROOT:
		{
			MT3_node subtree = mt3_AllocTree();
			
			for(auto& element : node.items())
			{
				parse(element.key().c_str(), element.value(), &subtree);
			}
			
			mt3_Append(object, subtree);
			mt3_Delete(&subtree);
			break;
		}
		
		default:
		{
			SP_ASSERT(node.is_array(), "Expected node to be array");
			MT3_node list = mt3_AllocList();
			
			for(SPsize i = 0; i < node.size(); i++)
			{
				parseList(node[i], &list);
			}
			
			mt3_Append(object, list);
			mt3_Delete(&list);
			break;
		}
	}
}

static MT3_tag getTag(JSON node)
{
	if(node.is_boolean())
	{
		return MT3_TAG_BYTE;
	}
	
	if(node.is_number())
	{
		return getNumberTag(node);
	}
	
	if(node.is_string())
	{
		return MT3_TAG_STRING;
	}
	
	if(node.is_object())
	{
		return MT3_TAG_ROOT;
	}
	
	if(node.is_array())
	{
		return getListTag(node);
	}
	
	return MT3_TAG_NULL;
}

static MT3_tag getNumberTag(JSON node)
{
	if(node.is_number_integer())
		return getIntegerTag(node.get<SPlong>());
	
	if(node.is_number_float())
		return getDecimalTag(node.get<SPdouble>());
	
	return MT3_TAG_NULL;
}

static MT3_tag getIntegerTag(SPlong value)
{
	if(value >= CHAR_MIN && value <= CHAR_MAX)
		return MT3_TAG_BYTE;
	
	if(value >= SHRT_MIN && value <= SHRT_MAX) 
		return MT3_TAG_SHORT;
	
	if(value >= INT_MIN && value <= INT_MAX)
		return MT3_TAG_INT;
	
	return MT3_TAG_LONG;
}

static MT3_tag getDecimalTag(SPdouble value)
{
	return (fabs((SPdouble)(SPfloat)value - value) < FLT_EPSILON * fabs(value)) ? MT3_TAG_FLOAT : MT3_TAG_DOUBLE;
}

static MT3_tag getListTag(JSON node)
{
	MT3_tag tag = MT3_TAG_NULL;
	if(node.is_array())
	{
		tag = MT3_TAG_LIST;
		if(node.size() > 0)
		{
			JSON front = node[0];
			
			if(front.is_boolean())
				return (MT3_tag)(tag | MT3_TAG_BYTE);
			
			if(front.is_number())
				return (MT3_tag)(tag | getNumberTag(front));
			
			if(front.is_string())
				return (MT3_tag)(tag | MT3_TAG_STRING);
			
			if(front.is_object())
				return (MT3_tag)(tag | MT3_TAG_ROOT);
		}
	}
	
	return tag;
}
