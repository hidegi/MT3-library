#ifndef SIMPLE_THREAD_IMPL_H_INCLUDED
#define SIMPLE_THREAD_IMPL_H_INCLUDED
#include "SP/cnc/thread.h"

namespace sp
{
	namespace detail
	{
		class ThreadLocalImpl
		{
			public:
					ThreadLocalImpl() = default;
				virtual ~ThreadLocalImpl() = default;

				virtual void  setValue(void* value) = 0;
				virtual void* getValue() const = 0;
		};
		class MutexImpl
		{
			public:
						 MutexImpl() = default;
				virtual ~MutexImpl() = default;

				virtual void lock() = 0;
				virtual void unlock() = 0;
		};

		class ThreadImpl
		{
			public:
				ThreadImpl(Thread* user) :
					m_user{user}
				{
				}

				virtual ~ThreadImpl(){}
				virtual void wait() = 0;
				virtual void terminate() = 0;

			protected:
				static void run(Thread* user);
				Thread* m_user;
		};
	}
}

#endif // THREAD_IMPL_H_INCLUDED
