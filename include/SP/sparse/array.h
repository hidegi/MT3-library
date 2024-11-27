#ifndef MT3_ARRAY_H
#define MT3_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif
#define MT3_LAST_POS -1

SP_API void mt3_AppendtByte(MT3_array array, SPbyte value);
SP_API void mt3_AppendShort(MT3_array array, SPshort value);
SP_API void mt3_AppendInt(MT3_array array, SPint value);
SP_API void mt3_AppendLong(MT3_array array, SPlong value);
SP_API void mt3_AppendFloat(MT3_array array, SPfloat value);
SP_API void mt3_AppendDouble(MT3_array array, SPdouble value);
SP_API void mt3_AppendTree(MT3_array array, const MT3_tree value);
SP_API void mt3_AppendString(MT3_array array, const SPchar* value);
SP_API void mt3_AppendByteArray(MT3_array array, const SPbyte* values);
SP_API void mt3_AppendShortArray(MT3_array array, const SPshort* values);
SP_API void mt3_AppendIntArray(MT3_array array, const SPint* values);
SP_API void mt3_AppendLongArray(MT3_array array, const SPlong* values);
SP_API void mt3_AppendFloatArray(MT3_array array, const SPfloat* values);
SP_API void mt3_AppendLongArray(MT3_array array, const SPfloat* values);
SP_API void mt3_AppendArray(MT3_array array, const MT3_array value);

SP_API SPbyte mt3_DeleteAt(MT3_array array, SPindex pos);

#ifdef __cplusplus
}
#endif
#endif