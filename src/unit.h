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
#ifndef SP_TEST_H
#define SP_TEST_H
#include "platform.h"
#include "types.h"
#include "asserts.h"

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wformat"
#endif
#ifndef SP_DEFAULT_PRECISION
#define SP_DEFAULT_PRECISION 15
#endif
#define SP_TEST_PASS 0
#define SP_TEST_FAIL 1
#define SP_TEST_ADD(action) spTestAdd(#action, action)
#define SP_TEST_INIT(c, v)								\
	if(!spTestInit((c), (v)))							\
    {													\
        SP_WARNING("Failed to initialize SP-test");		\
        return 1;										\
    }
	
#ifdef __cplusplus
extern "C" {
#endif

typedef void(*SPaction)();

SP_API SPbool spTestInit(SPint argc, SPchar** argv);
SP_API SPbool spTestAdd(const SPchar* name, SPaction action);
SP_API SPbool spTestDecimalEqual(SPdouble a, SPdouble b);
SP_API SPbool spTestDecimalEqual(SPdouble a, SPdouble b);
SP_API SPbool spTestRunAll();

SP_API SPbool spTestDecimalAlmostEqual(SPdouble a, SPdouble b, SPsize places);
SP_API SPbool spTestStringEqual(const SPchar* a, const SPchar* b);
SP_API SPbool spTestStringAlmostEqual(const SPchar* a, const SPchar* b, SPsize);

SP_API void spTestTerminate();

#ifdef __cplusplus
}
#endif
#endif