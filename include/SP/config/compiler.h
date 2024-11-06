#ifndef SIMPLE_COMPILER_H_INCLUDED
#define SIMPLE_COMPILER_H_INCLUDED
#if defined(__clang__)
#define SP_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUC__)
#define SP_COMPILER_GNUC
#elif defined(_MSC_VER)
#define SP_COMPILER_MSC
#endif

#ifdef SP_COMPILER_GNUC
#define SP_MAYBE_UNUSED __attribute__((__unused__))
#elif defined(SP_COMPILER_MSC)
#define SP_MAYBE_UNUSED __pragma(warning(suppress:4100))
#elif defined(SP_COMPILER_CLANG)
#define SP_MAYBE_UNUSED [[maybe_unused]]
#else
#define SP_MAYBE_UNUSED
#endif


#if defined(SP_PLATFORM_WINDOWS) && (defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC))
	#define SP_ALIGN_STACK __attribute__((__force_align_arg_pointer__))
#else
	#define SP_ALIGN_STACK
#endif
#endif // SIMPLE_COMPILER_H_INCLUDED
