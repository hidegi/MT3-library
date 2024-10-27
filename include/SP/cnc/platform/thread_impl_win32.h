#ifndef THREAD_IMPL_WIN32_H_INCLUDED
#define THREAD_IMPL_WIN32_H_INCLUDED
#include "SP/config.h"
#include "SP/cnc/thread_impl.h"

namespace sp
{
    namespace detail
    {
        void sleepImplWin32(SPint64 duration);
        class ThreadLocalImplWin32 final : public ThreadLocalImpl
        {
            private:
                friend class sp::ThreadLocal;
                ThreadLocalImplWin32();
               ~ThreadLocalImplWin32();

                virtual void  setValue(void* value) override;
                virtual void* getValue() const override;

                DWORD m_index;
        };

        class MutexImplWin32 final : public MutexImpl
        {
            private:
                friend class sp::Mutex;
                 MutexImplWin32();
                ~MutexImplWin32();

                virtual void lock() override;
                virtual void unlock() override;

                CRITICAL_SECTION m_section;
        };

        class ThreadImplWin32 final : public ThreadImpl
        {
            public:
                ThreadImplWin32(Thread* owner);
               ~ThreadImplWin32();

                virtual void wait() override;
                virtual void terminate() override;

            private:
                SP_ALIGN_STACK static unsigned int __stdcall entryPoint(void* userData);
                HANDLE m_thread;
                unsigned int m_thread_id;
        };
    }
}

#endif // THREAD_IMPL_WIN32_H_INCLUDED
