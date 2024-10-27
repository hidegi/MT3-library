#include "SP/cnc/thread_impl.h"

namespace sp
{
    namespace detail
    {
        void ThreadImpl::run(Thread* user)
        {
            user->run();
        }
    }
}
