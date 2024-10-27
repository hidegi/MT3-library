#ifndef SP_CPP11_BACKWARD_COMPAT
#define SP_CPP11_BACKWARD_COMPAT
#if __cplusplus < 201402L
#include <memory>
namespace std
{
    template<typename T, typename ... Args>
    std::unique_ptr<T> make_unique(Args&& ... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template<typename _IIter1, typename _IIter2, typename _BinaryPredicate>
    bool equal(_IIter1 __first1, _IIter1 __last1,
               _IIter2 __first2, _IIter2 __last2, _BinaryPredicate __binary_pred)
    {
        while (__first1 != __last1 && __first2 != __last2) {
            if (!__binary_pred(*__first1, *__first2))
                return false;
            ++__first1;
            ++__first2;
        }
        return __first1 == __last1 && __first2 == __last2;
    }
}
#endif // __cplusplus
#endif // MISC_H_INCLUDED
