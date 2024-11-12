template <typename T>
ThreadLocalPtr<T>::ThreadLocalPtr(T* value) :
ThreadLocal(value)
{
}

template <typename T>
T& ThreadLocalPtr<T>::operator *() const
{
	return *static_cast<T*>(getValue());
}

template <typename T>
T* ThreadLocalPtr<T>::operator ->() const
{
	return static_cast<T*>(getValue());
}

template <typename T>
ThreadLocalPtr<T>::operator T*() const
{
	return static_cast<T*>(getValue());
}

template <typename T>
ThreadLocalPtr<T>& ThreadLocalPtr<T>::operator =(T* value)
{
	setValue(value);
	return *this;
}

template <typename T>
ThreadLocalPtr<T>& ThreadLocalPtr<T>::operator =(const ThreadLocalPtr<T>& right)
{
	setValue(right.getValue());
	return *this;
}

namespace detail
{
	struct ThreadFunctor
	{
		virtual ~ThreadFunctor(){}
		virtual void run() = 0;
	};

	template<typename T>
	struct ThreadBaseFunctor : ThreadFunctor
	{
		ThreadBaseFunctor(T functor) : m_functor(functor){}
		virtual void run() override {m_functor();}
		T m_functor;
	};

	template<typename F, typename A>
	struct ThreadArgFunctor : ThreadFunctor
	{
		ThreadArgFunctor(F f, A a) : m_function(f), m_arg(a){}
		virtual void run() override {m_function(m_arg);}
		F m_function;
		A m_arg;
	};

	template<typename C>
	struct ThreadMemberFunctor : ThreadFunctor
	{
		ThreadMemberFunctor(void(C::*f)(), C* instance) : m_function(f), m_instance(instance){}
		virtual void run() override {(m_instance->*m_function)();}
		void(C::*m_function)();
		C* m_instance;
	};
}
template<typename F>
Thread::Thread(F f) :
	m_thread_impl{NULL},
	m_entry_point{new detail::ThreadBaseFunctor<F>(f)}
{
}

template<typename F, typename A>
Thread::Thread(F f, A a) :
	m_thread_impl{nullptr},
	m_entry_point{new detail::ThreadArgFunctor<F, A>(f, a)}
{
}

template<typename C>
Thread::Thread(void(C::*f)(), C* instance) :
	m_thread_impl{nullptr},
	m_entry_point{new detail::ThreadMemberFunctor<C>(f, instance)}
{
}




