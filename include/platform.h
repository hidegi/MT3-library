#ifndef MT3_PLATFORM_H
#define MT3_PLATFORM_H

#define MT3_VERSION_MAJOR 1
#define MT3_VERSION_MINOR 1
#define MT3_VERSION_PATCH 1


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
