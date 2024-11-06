#ifndef NO_MOVE_H_INCLUDED
#define NO_MOVE_H_INCLUDED

namespace sp
{
	class NonMovable
	{
		public:
			NonMovable(){}
		       ~NonMovable(){}
			NonMovable(NonCopyable&&) = delete;
			NonMovable& operator=(NonCopyable&&) = delete;
	};
}

#endif // NO_MOVE_H_INCLUDED
