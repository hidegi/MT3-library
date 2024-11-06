#include "SP/cnc/platform/thread_impl_posix.h"
#include <errno.h>
#include <time.h>

namespace sp
{
	namespace detail
	{
		ThreadLocalImplPosix::ThreadLocalImplPosix() :
			m_key{0}
		{
			pthread_key_create(&m_key, NULL);
			setValue(NULL);
		}

		void ThreadLocalImplPosix::setValue(void* value)
		{
			pthread_setspecific(m_key, value);
		}

		void* ThreadLocalImplPosix::getValue() const
		{
			return pthread_getspecific(m_key);
		}

		ThreadLocalImplPosix::~ThreadLocalImplPosix()
		{
			pthread_key_delete(m_key);
		}

		MutexImplPosix::MutexImplPosix() :
			m_mutex{0}
		{
			pthread_mutexattr_t attributes;
			pthread_mutexattr_init(&attributes);
			pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&m_mutex, NULL);
		}

		MutexImplPosix::~MutexImplPosix()
		{
			pthread_mutex_destroy(&m_mutex);
		}

		void MutexImplPosix::lock()
		{
			pthread_mutex_lock(&m_mutex);
		}

		void MutexImplPosix::unlock()
		{
			pthread_mutex_unlock(&m_mutex);
		}

		ThreadImplPosix::ThreadImplPosix(Thread* user) :
			ThreadImpl(user),
			m_is_active{false}
		{
			m_is_active = pthread_create(&m_thread, NULL, &ThreadImplPosix::entryPoint, user) == 0;
			if(!m_is_active)
				SP_WARNING("Failed to create pthread");
		}

		ThreadImplPosix::~ThreadImplPosix()
		{
		}
		void ThreadImplPosix::wait()
		{
			if(m_is_active)
			{
				SP_ASSERT(pthread_equal(pthread_self(), m_thread) == 0, "Thread cannot wait for itself");
				pthread_join(m_thread, NULL);
			}
		}

		void ThreadImplPosix::terminate()
		{
			if(m_is_active)
			{
				pthread_cancel(m_thread);
			}
		}

		void* ThreadImplPosix::entryPoint(void* userData)
		{
			pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
			ThreadImpl::run(static_cast<Thread*>(userData));

			return NULL;
		}
	}
}
