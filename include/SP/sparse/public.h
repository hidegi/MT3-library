#ifndef MT3_PUBLIC_INTERFACE_H
#define MT3_PUBLIC_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	/// plain-type tags
	MT3_TAG_NULL 	= 0,
	MT3_TAG_ROOT    = 1,
	MT3_TAG_BYTE    = 2,
	MT3_TAG_SHORT   = 3,
	MT3_TAG_INT     = 4,
	MT3_TAG_LONG    = 5,
	MT3_TAG_FLOAT 	= 6,
	MT3_TAG_DOUBLE  = 7,
	MT3_TAG_STRING 	= 8,
	MT3_TAG_list	= 0x80,

	/// sub-tree tag
	MT3_TAG_ROOT_list	 = MT3_TAG_list | MT3_TAG_ROOT,

	/// list-type tags
	MT3_TAG_BYTE_list   = MT3_TAG_list | MT3_TAG_BYTE,
	MT3_TAG_SHORT_list  = MT3_TAG_list | MT3_TAG_SHORT,
	MT3_TAG_INT_list    = MT3_TAG_list | MT3_TAG_INT,
	MT3_TAG_LONG_list   = MT3_TAG_list | MT3_TAG_LONG,
	MT3_TAG_FLOAT_list  = MT3_TAG_list | MT3_TAG_FLOAT,
	MT3_TAG_DOUBLE_list = MT3_TAG_list | MT3_TAG_DOUBLE,
	MT3_TAG_STRING_list = MT3_TAG_list | MT3_TAG_STRING
} MT3_tag;

typedef enum
{
	MT3_STATUS_OK = 0, 			/// no errors
	MT3_STATUS_NO_MEMORY, 		/// out of memory
	MT3_STATUS_READ_ERROR,		/// read error
	MT3_STATUS_WRITE_ERROR,		/// write error
	MT3_STATUS_BAD_NAME,		/// bad name, either taken, empty or null
	MT3_STATUS_BAD_VALUE,		/// invalid value
	MT3_STATUS_BAD_TAG,			/// invalid tag
} MT3_status;

struct _MT3_node;
typedef struct _MT3_node* MT3_node;

SP_API MT3_node mt3_AllocTree();
SP_API MT3_node mt3_Alloclist();

SP_API MT3_node mt3_CopyTree(const MT3_node tree);
SP_API MT3_node mt3_Copylist(const MT3_node list);
SP_API MT3_tag mt3_GetTag(const MT3_node node);

/*<==========================================================>*
 *  tree interface
 *<==========================================================>*/
SP_API SPbool mt3_IsTree(const MT3_node node);
SP_API void mt3_InsertTree(MT3_node* tree, const char* name, const MT3_node value);
SP_API void mt3_InsertByte(MT3_node* tree, const char* name, SPbyte value);
SP_API void mt3_InsertShort(MT3_node* tree, const char* name, SPshort value);
SP_API void mt3_InsertInt(MT3_node* tree, const char* name, SPint value);
SP_API void mt3_InsertLong(MT3_node* tree, const char* name, SPlong value);
SP_API void mt3_InsertFloat(MT3_node* tree, const char* name, SPfloat value);
SP_API void mt3_InsertDouble(MT3_node* tree, const char* name, SPdouble value);
SP_API void mt3_InsertString(MT3_node* tree, const char* name, const SPchar* value);
SP_API void mt3_InsertByteList(MT3_node* tree, const char* name, SPsize length, const SPbyte* values);
SP_API void mt3_InsertShortList(MT3_node* tree, const char* name, SPsize length, const SPshort* values);
SP_API void mt3_InsertIntList(MT3_node* tree, const char* name, SPsize length, const SPint* values);
SP_API void mt3_InsertLongList(MT3_node* tree, const char* name, SPsize length, const SPlong* values);
SP_API void mt3_InsertFloatList(MT3_node* tree, const char* name, SPsize length, const SPfloat* values);
SP_API void mt3_InsertDoubleList(MT3_node* tree, const char* name, SPsize length, const SPdouble* values);
SP_API void mt3_InsertStringList(MT3_node* tree, const char* name, SPsize length, const SPchar** values);
SP_API void mt3_InsertList(MT3_node* tree, const MT3_node list);

SP_API void mt3_SetByte(MT3_node tree, const char* name, SPbyte value);
SP_API void mt3_SetShort(MT3_node tree, const char* name, SPshort value);
SP_API void mt3_SetInt(MT3_node tree, const char* name, SPint value);
SP_API void mt3_SetLong(MT3_node tree, const char* name, SPlong value);
SP_API void mt3_SetFloat(MT3_node tree, const char* name, SPfloat value);
SP_API void mt3_SetDouble(MT3_node tree, const char* name, SPdouble value);
SP_API void mt3_SetString(MT3_node tree, const char* name, const SPchar* value);

SP_API SPbyte mt3_GetByte(const MT3_node tree, const SPchar* name);
SP_API SPshort mt3_GetShort(const MT3_node tree, const SPchar* name);
SP_API SPint mt3_GetInt(const MT3_node tree, const SPchar* name);
SP_API SPlong mt3_GetLong(const MT3_node tree, const SPchar* name);
SP_API SPfloat mt3_GetFloat(const MT3_node tree, const SPchar* name);
SP_API SPdouble mt3_GetDouble(const MT3_node tree, const SPchar* name);
SP_API const SPchar* mt3_GetString(const MT3_node tree, const SPchar* name);
SP_API MT3_node* mt3_GetTree(const MT3_node tree, const SPchar* name);
SP_API MT3_node* mt3_Getlist(const MT3_node tree, const SPchar* name);

SP_API SPbool mt3_Remove(MT3_node* tree, const SPchar* name);
SP_API SPbool mt3_VerifyRBT(const MT3_tree rbt);
/*<==========================================================>*
 *  list interface
 *<==========================================================>*/
SP_API SPbool mt3_IsList(const MT3_node node);
SP_API void mt3_AppendByte(MT3_node* list, SPbyte value);
SP_API void mt3_AppendShort(MT3_node* list, SPshort value);
SP_API void mt3_AppendInt(MT3_node* list, SPint value);
SP_API void mt3_AppendLong(MT3_node* list, SPlong value);
SP_API void mt3_AppendFloat(MT3_node* list, SPfloat value);
SP_API void mt3_AppendDouble(MT3_node* list, SPdouble value);
SP_API void mt3_AppendTree(MT3_node* list, SPsize length, const MT3_node value);
SP_API void mt3_AppendString(MT3_node* list, const SPchar* value);
SP_API void mt3_AppendByteList(MT3_node* list, SPsize length, const SPbyte* values);
SP_API void mt3_AppendShortList(MT3_node* list, SPsize length, const SPshort* values);
SP_API void mt3_AppendIntList(MT3_node* list, SPsize length, const SPint* values);
SP_API void mt3_AppendLongList(MT3_node* list, SPsize length, const SPlong* values);
SP_API void mt3_AppendFloatList(MT3_node* list, SPsize length, const SPfloat* values);
SP_API void mt3_AppendDoubleList(MT3_node* list, SPsize length, const SPdouble* values);
SP_API void mt3_AppendStringList(MT3_node* list, SPsize length, const SPchar** values);
SP_API void mt3_AppendList(MT3_node* list, const MT3_node value);

SP_API void mt3_SetByteAt(MT3_node list, SPindex index, SPbyte value);
SP_API void mt3_SetShortAt(MT3_node list, SPindex index, SPshort value);
SP_API void mt3_SetIntAt(MT3_node list, SPindex index, SPint value);
SP_API void mt3_SetLongAt(MT3_node list, SPindex index, SPlong value);
SP_API void mt3_SetFloatAt(MT3_node list, SPindex index, SPfloat value);
SP_API void mt3_SetDoubleAt(MT3_node list, SPindex index, SPdouble value);
SP_API void mt3_SetStringAt(MT3_node list, SPindex index, const SPchar* value);

SP_API SPbyte mt3_GetByteAt(const MT3_node list, SPindex index);
SP_API SPshort mt3_GetShortAt(const MT3_node list, SPindex index);
SP_API SPint mt3_GetIntAt(const MT3_node list, SPindex index);
SP_API SPlong mt3_GetLongAt(const MT3_node list, SPindex index);
SP_API SPfloat mt3_GetFloatAt(const MT3_node list, SPindex index);
SP_API SPdouble mt3_GetDoubleAt(const MT3_node list, SPindex index);
SP_API const SPchar* mt3_GetStringAt(const MT3_node list, SPindex index);
SP_API MT3_node* mt3_GetTreeAt(const MT3_node list, SPindex index);
SP_API MT3_node* mt3_GetlistAt(const MT3_node list, SPindex index);

//converts a contiguous list in memory to a linked list..
SP_API MT3_node mt3_Tolist(MT3_tag tag, SPsize length, const void* data);
SP_API MT3_node mt3_Next(const MT3_node n);
SP_API MT3_node mt3_Last(const MT3_node n);

SP_API void mt3_RemoveAt(MT3_node list, SPindex pos);


/*<==========================================================>*/
SP_API SPbuffer mt3_WriteBinary(const MT3_node node);
SP_API MT3_node mt3_ReadBinary(SPbuffer buffer);
SP_API void mt3_Delete(MT3_node* node);

SP_API MT3_status mt3_GetLastError();
SP_API const char* mt3_GetErrorInfo(MT3_status status);
#ifdef __cplusplus
}
#endif
#endif