#ifndef SP_TREE_H
#define SP_TREE_H
#include "SP/config.h"
#include "SP/sparse/mt3.h"
#include <string>

namespace sp
{
	class SP_API BTO final
	{
		public:
			BTO();
		   ~BTO();
		   
			bool loadFromFile(const std::string& path);
			bool writeToFile(const std::string& path);
			
			void insertByte(const std::string& name, SPbyte data);
			void insertShort(const std::string& name, SPshort data);
			void insertInt(const std::string& name, SPint data);
			void insertLong(const std::string& name, SPlong data);
			void insertFloat(const std::string& name, SPfloat data);
			void insertDouble(const std::string& name, SPdouble data);
			void insertString(const std::string& name, const std::string& data);
			void insertTree(const std::string& name);
			
			SPbyte getByte(const std::string& name);
			SPshort getShort(const std::string& name);
			SPint getInt(const std::string& name);
			SPlong getLong(const std::string& name);
			SPfloat getFloat(const std::string& name);
			SPdouble getDouble(const std::string& name);
			std::string getString(const std::string& name);
			
			bool setByte(const std::string& name, SPbyte data);
			bool setShort(const std::string& name, SPshort data);
			bool setInt(const std::string& name, SPint data);
			bool setLong(const std::string& name, SPlong data);
			bool setFloat(const std::string& name, SPfloat data);
			bool setDouble(const std::string& name, SPdouble data);
			bool setString(const std::string& name, const std::string& data);
			bool remove(const std::string& name);
			
			void debugPrint();
		private:
			MT3_tree m_tree;
			
	};
}
#endif