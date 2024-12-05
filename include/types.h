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
#ifndef SP_TYPES_H
#define SP_TYPES_H
typedef float SPfloat;
typedef double SPdouble;

typedef char SPchar;
typedef unsigned char SPuchar;
typedef signed char SPint8;
typedef unsigned char SPuint8;
typedef SPint8 SPbyte;
typedef SPuint8 SPubyte;

typedef signed short SPint16;
typedef unsigned short SPuint16;
typedef SPint16 SPshort;
typedef SPuint16 SPushort;

#define SP_FALSE 0
#define SP_TRUE 1
typedef signed int SPint32;
typedef unsigned int SPuint32;
typedef SPint32 SPint;
typedef SPuint32 SPuint;
typedef SPint SPbool;

#if defined(SP_MSC_VER)
typedef signed __int64 SPint64;
typedef unsigned __int64 SPuint64;
#else
typedef signed long long SPint64;
typedef unsigned long long SPuint64;
#endif // defined
typedef SPint64 SPlong;
typedef SPuint64 SPulong;

typedef SPulong SPsize;
typedef SPsize SPindex;
typedef SPulong SPhash;
#endif