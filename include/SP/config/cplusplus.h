#ifndef SIMPLE_CPLUSPLUS_H_INCLUDED
#define SIMPLE_CPLUSPLUS_H_INCLUDED

#ifdef __cplusplus
#if defined(__cpp_constexpr) && (__cpp_constexpr >= 201304L)
    #define SP_CONSTEXPR constexpr
#else
    #define SP_CONSTEXPR inline
#endif // defined
#endif // __cplusplus

#endif // CPLUSPLUS_H_INCLUDED
