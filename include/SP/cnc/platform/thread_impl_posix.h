#ifndef SIMPLE_THREAD_IMPL_POSIX_H_INCLUDED
#define SIMPLE_THREAD_IMPL_POSIX_H_INCLUDED
#include "SP/cnc/thread_impl.h"
#include <pthread.h>

namespace sp
{
	namespace detail
	{
		void sleepImplPosix(SPint64 duration);
		class ThreadLocalImplPosix final : public ThreadLocalImpl
		{
			private:
				friend class sp::ThreadLocal;
				ThreadLocalImplPosix();
			       ~ThreadLocalImplPosix();

				virtual void  setValue(void* value) override;
				virtual void* getValue() const override;

				pthread_key_t m_key;
		};

		class MutexImplPosix final : public MutexImpl
		{
			private:
				friend class sp::Mutex;
				 MutexImplPosix();
				~MutexImplPosix();

				virtual void lock() override;
				virtual void unlock() override;

				pthread_mutex_t m_mutex;
		};

		class ThreadImplPosix final : public ThreadImpl
		{
			public:
				ThreadImplPosix(Thread* owner);
			   ~ThreadImplPosix();

				virtual void wait() override;
				virtual void terminate() override;

			private:
				static void* entryPoint(void* userData);
				pthread_t m_thread;
				bool m_is_active;
		};
	}
}

#endif // THREAD_IMPL_POSIX_H_INCLUDED
