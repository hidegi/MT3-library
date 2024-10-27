#ifndef SIMPL_THREAD_H
#define SIMPL_THREAD_H
#include "SP/config.h"
#include "SP/base/no_copy.h"

namespace sp
{
    namespace detail
    {
        class MutexImpl;
        class ThreadLocalImpl;
        class ThreadImpl;
        struct ThreadFunctor;
    }

    //class for thread local objects..
    class SP_API ThreadLocal : public NonCopyable
    {
        public:
            ThreadLocal(void* value = NULL);
           ~ThreadLocal();

            void  setValue(void* value);
            void* getValue() const;

        private:
            detail::ThreadLocalImpl* m_thread_local_impl;
    };

    template<typename T>
    class SP_API ThreadLocalPtr : private ThreadLocal
    {
        public:
            ThreadLocalPtr(T* value = NULL);
            T& operator*() const;
            T* operator->() const;
            operator T*() const;
            ThreadLocalPtr<T>& operator=(T* value);
            ThreadLocalPtr<T>& operator=(const ThreadLocalPtr<T>& other);
    };

    class SP_API Mutex : public NonCopyable
    {
        public:
            Mutex();
           ~Mutex();

            void lock();
            void unlock();

        private:
            detail::MutexImpl* m_mutex_impl;
    };

    class SP_API Thread : public NonCopyable
    {
        public:
            template<typename F>
            Thread(F f);

            template<typename F, typename A>
            Thread(F f, A a);

            template<typename C>
            Thread(void(C::*f)(), C* cls);

           ~Thread();

            void launch();
            void wait();
            void terminate();

        private:
            friend class detail::ThreadImpl;
            void run();

            detail::ThreadImpl* m_thread_impl;
            detail::ThreadFunctor* m_entry_point;
    };

    class SP_API Lock
    {
        public:
            Lock(sp::Mutex& mutex);
           ~Lock();

            sp::Mutex& m_mutex;
    };
#include "SP/cnc/thread.inl"
}
#endif // SIMPL_MUTEX_H
