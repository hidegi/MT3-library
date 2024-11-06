#include <windows.h>
#include <process.h>
#include "SP/cnc/platform/thread_impl_win32.h"
namespace sp
{
	namespace detail
	{
		MutexImplWin32::MutexImplWin32()
		{
			InitializeCriticalSection(&m_section);
		}
		void MutexImplWin32::lock()
		{
			EnterCriticalSection(&m_section);
		}

		void MutexImplWin32::unlock()
		{
			LeaveCriticalSection(&m_section);
		}

		MutexImplWin32::~MutexImplWin32()
		{
			DeleteCriticalSection(&m_section);
		}

		ThreadLocalImplWin32::ThreadLocalImplWin32() :
			m_index{TLS_OUT_OF_INDEXES}
		{
			m_index = TlsAlloc();
			SP_ASSERT(m_index != TLS_OUT_OF_INDEXES, "Failed to create TLS");
			setValue(NULL);
		}

		void* ThreadLocalImplWin32::getValue() const
		{
			SP_ASSERT(m_index != TLS_OUT_OF_INDEXES, "Failed get TLS value");
			return TlsGetValue(m_index);
		}

		void ThreadLocalImplWin32::setValue(void* value)
		{
			SP_ASSERT(m_index != TLS_OUT_OF_INDEXES, "Failed to set TLS value");
			TlsSetValue(m_index, value);
		}

		ThreadLocalImplWin32::~ThreadLocalImplWin32()
		{
			if(m_index != TLS_OUT_OF_INDEXES)
				TlsFree(m_index);
		}

		ThreadImplWin32::ThreadImplWin32(Thread* user) : ThreadImpl(user)
		{
			m_thread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, &ThreadImplWin32::entryPoint, user, 0, &m_thread_id));
		}

		ThreadImplWin32::~ThreadImplWin32()
		{
			if(m_thread)
				CloseHandle(m_thread);
		}

		void ThreadImplWin32::wait()
		{
			if(m_thread)
			{
				SP_ASSERT(m_thread_id != GetCurrentThreadId(), "Thread cannot wait for itself");
				WaitForSingleObject(m_thread, INFINITE);
			}
		}

		void ThreadImplWin32::terminate()
		{
			if(m_thread)
				TerminateThread(m_thread, 0);
		}

		unsigned int __stdcall ThreadImplWin32::entryPoint(void* userData)
		{
			ThreadImpl::run(static_cast<Thread*>(userData));
			_endthreadex(0);
			return 0;
		}
	}
}
