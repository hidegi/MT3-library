#ifndef MT3_ARRAY_H
#define MT3_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif
#define MT3_LAST_POS -1

SP_API MT3_array mt3_AllocArray();

SP_API void mt3_InsertTree(MT3_tree tree, const char* name, const MT3_tree value);
SP_API void mt3_InsertByte(MT3_tree tree, const char* name, SPbyte value);
SP_API void mt3_InsertShort(MT3_tree tree, const char* name, SPshort value);
SP_API void mt3_InsertInt(MT3_tree tree, const char* name, SPint value);
SP_API void mt3_InsertLong(MT3_tree tree, const char* name, SPlong value);
SP_API void mt3_InsertFloat(MT3_tree tree, const char* name, SPfloat value);
SP_API void mt3_InsertDouble(MT3_tree tree, const char* name, SPdouble value);
SP_API void mt3_InsertString(MT3_tree tree, const char* name, const SPchar* value);
SP_API void mt3_InsertByteArray(MT3_tree tree, const char* name, SPsize length, const SPbyte* values);
SP_API void mt3_InsertShortArray(MT3_tree tree, const char* name, SPsize length, const SPshort* values);
SP_API void mt3_InsertIntArray(MT3_tree tree, const char* name, SPsize length, const SPint* values);
SP_API void mt3_InsertLongArray(MT3_tree tree, const char* name, SPsize length, const SPlong* values);
SP_API void mt3_InsertFloatArray(MT3_tree tree, const char* name, SPsize length, const SPfloat* values);
SP_API void mt3_InsertDoubleArray(MT3_tree tree, const char* name, SPsize length, const SPdouble* values);
SP_API void mt3_InsertStringArray(MT3_tree tree, const char* name, SPsize length, const SPchar** values);
SP_API void mt3_InsertArray(MT3_tree tree, const MT3_array array);

SP_API void mt3_AppendByte(MT3_array array, SPbyte value);
SP_API void mt3_AppendShort(MT3_array array, SPshort value);
SP_API void mt3_AppendInt(MT3_array array, SPint value);
SP_API void mt3_AppendLong(MT3_array array, SPlong value);
SP_API void mt3_AppendFloat(MT3_array array, SPfloat value);
SP_API void mt3_AppendDouble(MT3_array array, SPdouble value);
SP_API void mt3_AppendTree(MT3_array array, SPsize length, const MT3_tree value);
SP_API void mt3_AppendString(MT3_array array, const SPchar* value);
SP_API void mt3_AppendByteArray(MT3_array array, SPsize length, const SPbyte* values);
SP_API void mt3_AppendShortArray(MT3_array array, SPsize length, const SPshort* values);
SP_API void mt3_AppendIntArray(MT3_array array, SPsize length, const SPint* values);
SP_API void mt3_AppendLongArray(MT3_array array, SPsize length, const SPlong* values);
SP_API void mt3_AppendFloatArray(MT3_array array, SPsize length, const SPfloat* values);
SP_API void mt3_AppendDoubleArray(MT3_array array, SPsize length, const SPdouble* values);
SP_API void mt3_AppendStringArray(MT3_array array, SPsize length, const SPchar** values);
SP_API void mt3_AppendArray(MT3_array array, const MT3_array value);

SP_API MT3_array mt3_ToArray(MT3_tag tag, SPsize length, const void* data);

SP_API void mt3_DeleteAt(MT3_array array, SPindex pos);
SP_API void mt3_FreeArray(MT3_array* array);

#ifdef __cplusplus
}
#endif
#endif