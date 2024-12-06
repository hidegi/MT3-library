/****************************************************************************
 * Copyright (c) 2024 Hidegi
 *
 * This software is provided ‘as-is’, without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ****************************************************************************/
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
	SPubyte buffer[8];
	for(SPsize i = 0; i < 8; i++)
	{
		buffer[i] = (hash >> (i * 8)) & 0xFF;
	}
	
	SPhash output = 0;
	memcpy(&output, buffer, sizeof(SPbyte) * 8);
	return output;
}
