#ifndef NO_COPY_H_INCLUDED
#define NO_COPY_H_INCLUDED

namespace sp
{
	class NonCopyable
	{
		public:
			NonCopyable(){}
		   ~NonCopyable(){}
			NonCopyable(const NonCopyable&) = delete;
			NonCopyable& operator=(const NonCopyable&) = delete;
	};
}

#endif // NO_COPY_H_INCLUDED
