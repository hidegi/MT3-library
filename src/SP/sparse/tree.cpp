#include "SP/sparse/tree.h"
#define MT3_INSERT_DATA(fn)\
		do\
		{\
			std::string domain = extractDomain(name);\
			std::string member = extractMember(name);\
			if(domain == member)\
			{\
				fn(&m_tree, name.c_str(), data);\
			}\
			else\
			{\
				MT3_tree* root = deepSearch(&m_tree, domain);\
				if(root)\
				{\
					fn(root, member.c_str(), data);\
				}\
			}\
		} while(0);
		
namespace
{
	// assuming that access-string has form of "a.b.c.etc.."
	/*
	 *	1. if empty string return head, otherwise,
	 *	1. extract first token
	 *		if no separator is found, then return head..
	 *
	 *	2. search for a child with such token..
	 *		if no such child exists, return NULL..
	 *	
	 *	3. deep search with remaining tokens..
	 */
	MT3_tree* deepSearch(const MT3_tree* head, const std::string& token)
	{
		// if it is empty, then return head..
		if(!head)
			return NULL;
		
		if(token.empty())
		{
			return const_cast<MT3_tree*>(head);
		}
		
		auto pos = token.find_first_of('.');
		if(pos == std::string::npos)
		{
			return mt3_GetTree(*head, token.c_str());
		}
		
		std::string branch = token.substr(0, pos);
		MT3_tree* child = mt3_GetTree(*head, branch.c_str());
		if(!child)
		{
			return NULL;
		}
		std::string children = token.substr(pos + 1, token.size() - pos);
		return deepSearch(child, children);
	}
	
	std::string extractMember(const std::string& str)
	{
		auto dot = str.find_last_of(".");
		if(dot == std::string::npos)
			return str;
		
		return str.substr(dot + 1, str.size());
	}

	std::string extractDomain(const std::string& str)
	{
		auto dot = str.find_last_of(".");
		if(dot == std::string::npos)
			return str;
		
		return str.substr(0, dot);
	}
}

namespace sp
{
	BTO::BTO() :
		m_tree{NULL}
	{
	}
	
	BTO::~BTO()
	{
		mt3_FreeTree(&m_tree);
	}
	
	void BTO::debugPrint()
	{
		mt3_PrintTree(m_tree);
	}

	void BTO::insertByte(const std::string& name, SPbyte data)
	{
		MT3_INSERT_DATA(mt3_InsertByte);
	}
	
	void BTO::insertShort(const std::string& name, SPshort data)
	{
		MT3_INSERT_DATA(mt3_InsertShort);
	}
	
	void BTO::insertInt(const std::string& name, SPint data)
	{
		MT3_INSERT_DATA(mt3_InsertInt);
	}
	
	void BTO::insertLong(const std::string& name, SPlong data)
	{
		MT3_INSERT_DATA(mt3_InsertLong);
	}
	
	void BTO::insertFloat(const std::string& name, SPfloat data)
	{
		MT3_INSERT_DATA(mt3_InsertFloat);
	}
	
	void BTO::insertDouble(const std::string& name, SPdouble data)
	{
		MT3_INSERT_DATA(mt3_InsertDouble);
	}
	
	void BTO::insertString(const std::string& name, const std::string& str)
	{
		const char* data = str.c_str();
		MT3_INSERT_DATA(mt3_InsertString);
	}
	
	void BTO::insertTree(const std::string& name)
	{
		MT3_tree data = NULL;
		MT3_INSERT_DATA(mt3_InsertTree);
	}
	
	SPbyte BTO::getByte(const std::string& name)
	{
		
	}
	/*
	SPshort getShort(const std::string& name);
	SPint getInt(const std::string& name);
	SPlong getLong(const std::string& name);
	SPfloat getFloat(const std::string& name);
	SPdouble getDouble(const std::string& name);
	std::string getString(const std::string& name);
	*/
}