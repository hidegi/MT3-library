#include "SP/cnc/thread.h"
#if defined(SP_PLATFORM_WINDOWS)
	#include "SP/cnc/platform/thread_impl_win32.h"
	typedef sp::detail::ThreadLocalImplWin32 ThreadLocalImplPlatform;
	typedef sp::detail::MutexImplWin32 MutexImplPlatform;
	typedef sp::detail::ThreadImplWin32 ThreadImplPlatform;
#elif defined(SP_PLATFORM_LINUX) || defined(SP_PLATFORM_MACOS)
	#include "SP/cnc/platform/thread_impl_posix.h"
	typedef sp::detail::ThreadLocalImplPosix ThreadLocalImplPlatform;
	typedef sp::detail::MutexImplPosix MutexImplPlatform;
	typedef sp::detail::ThreadImplPosix ThreadImplPlatform;
#else
#error Unknown platform to build thread API
#endif

namespace sp
{
	ThreadLocal::ThreadLocal(void* value)
	{
		m_thread_local_impl = new ThreadLocalImplPlatform;
		m_thread_local_impl->setValue(value);
	}

	void ThreadLocal::setValue(void* value)
	{
		SP_ASSERT(m_thread_local_impl != nullptr, "Thread local unavailable");
		m_thread_local_impl->setValue(value);
	}

	void* ThreadLocal::getValue() const
	{
		SP_ASSERT(m_thread_local_impl != nullptr, "Thread local unavailable");
		return m_thread_local_impl->getValue();
	}

	ThreadLocal::~ThreadLocal()
	{
		SP_ASSERT(m_thread_local_impl != nullptr, "Thread local unavailable");
		delete m_thread_local_impl;
		m_thread_local_impl = nullptr;
	}

	Mutex::Mutex() :
		m_mutex_impl{nullptr}
	{
		m_mutex_impl = new MutexImplPlatform;
		SP_ASSERT(m_mutex_impl != nullptr, "Mutex unavailable");
	}

	void Mutex::lock()
	{
		m_mutex_impl->lock();
	}

	void Mutex::unlock()
	{
		m_mutex_impl->unlock();
	}

	Mutex::~Mutex()
	{
		delete m_mutex_impl;
	}

	Lock::Lock(sp::Mutex& mutex) :
		m_mutex{mutex}
	{
		m_mutex.lock();
	}
	Lock::~Lock()
	{
		m_mutex.unlock();
	}

	Thread::~Thread()
	{
		wait();
		delete m_thread_impl;
	}

	void Thread::launch()
	{
		wait();
		m_thread_impl = new ThreadImplPlatform(this);
	}

	void Thread::wait()
	{
		if(m_thread_impl)
		{
			m_thread_impl->wait();
			delete m_thread_impl;
			m_thread_impl = nullptr;
		}
	}

	void Thread::terminate()
	{
		SP_ASSERT(m_thread_impl != nullptr, "Thread unavailable");
		m_thread_impl->terminate();
		delete m_thread_impl;
		m_thread_impl = nullptr;
	}

	void Thread::run()
	{
		m_entry_point->run();
	}
}
