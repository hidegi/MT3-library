#include "SP/sparse/tree.h"

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
	MT3_tree deepSearch(const MT3_tree head, const std::string& token)
	{
		if(!head)
			return NULL;
		
		if(token.empty())
		{
			return head;
		}
		
		auto pos = token.find_first_of('.');
		if(pos == std::string::npos)
		{
			MT3_tree* t = mt3_GetTree(head, token.c_str());
			return t ? *t : NULL;
		}
		
		std::string branch = token.substr(0, pos);
		MT3_tree* child = mt3_GetTree(head, branch.c_str());
		if(!child)
		{
			return NULL;
		}
		return deepSearch(*child, token.substr(pos + 1, token.size() - pos));
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
	
	void insertByte(const std::string& name, SPbyte data)
	{
	}
	
	void insertShort(const std::string& name, SPshort data)
	{
	}
	
	void insertInt(const std::string& name, SPint data)
	{
	}
	
	void insertLong(const std::string& name, SPlong data)
	{
	}
	
	void insertFloat(const std::string& name, SPfloat data)
	{
	}
	
	void insertDouble(const std::string& name, SPdouble data)
	{
	}
	
	void insertString(const std::string& name, const std::string& data)
	{
	}
	
	void insertTree(const std::string& name)
	{
	}
}