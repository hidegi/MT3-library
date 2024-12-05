/*************************************************************************************************
 * MT3 1.1 - MoTree Library
 * A lightweight library for serializing and deserializing BTOs (Binary Tree Objects).
 * 
 * Copyright (c) 2024 Hidegi
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *************************************************************************************************/
#ifndef MT3_PLATFORM_H
#define MT3_PLATFORM_H

#define MT3_VERSION_MAJOR 1
#define MT3_VERSION_MINOR 1
#define MT3_VERSION_PATCH 0


/* #undef MT3_PRINT_OUTPUT_DEBUG */
#define MT3_HAVE_BST_MAJOR_INCLINED
/* #undef SP_STATIC_BUILD */
/* #undef SP_PLATFORM_WINDOWS */
#define SP_PLATFORM_MACOS
/* #undef SP_PLATFORM_LINUX */

#ifndef SP_STATIC_BUILD
	#if defined(SP_PLATFORM_WINDOWS)
		#define SP_API __declspec(dllexport)
	#else
		#define SP_API __attribute__((visibility("default")))
	#endif
#else
	#define SP_API
#endif

#if defined(__clang__)
#define SP_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUC__)
#define SP_COMPILER_GNUC
#elif defined(_MSC_VER)
#define SP_COMPILER_MSC
#endif

#endif
