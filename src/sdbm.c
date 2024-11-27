#include "internal.h"

static SPhash _mt3_sdbm_impl(const SPchar *str)
{
	SPhash hash = 0;
	int c;
	while ((c = *str++))
		hash = c + (hash << 6) + (hash << 16) - hash; // hash * 65599 + c

	return hash;
}

SPhash _mt3_sdbm(const SPchar* str)
{
	SPhash hash = _mt3_sdbm_impl(str);
	SPubyte buffer[4];

	for(SPsize i = 0; i < 8; i++)
	{
		buffer[i] = (hash >> (i * 8)) & 0xFF;
	}
	SPhash output = 0;
	memcpy(&output, buffer, sizeof(SPbyte) * 4);
	return output;
}