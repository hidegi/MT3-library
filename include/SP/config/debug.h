#ifndef SP_DEBUG_H_INCLUDED
#define SP_DEBUG_H_INCLUDED

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef NDEBUG
#define SP_NO_DEBUG
#endif

#define SP_OUTPUT(stream, notif, msg, ...)          \
{                                                   \
    fprintf((stream), "%-16s", (notif));            \
    fprintf((stream), msg, ##__VA_ARGS__);          \
    fprintf((stream), "..\n");                      \
}

#define SP_WARNING(msg, ...)                        \
    SP_OUTPUT(stderr, "[SP WARNING]:",              \
              msg, ##__VA_ARGS__)                   \

#define SP_ASSERT(c, msg, ...)                      \
if(!(c))                                            \
{                                                   \
    SP_WARNING(msg, ##__VA_ARGS__)                  \
    assert(c);                                      \
}

#ifndef SP_NO_DEBUG
#define SP_INFO(msg, ...)                           \
    SP_OUTPUT(stdout, "[SP INFO]:",                 \
              msg, ##__VA_ARGS__)                   \

#define SP_DEBUG(msg, ...)                          \
    SP_OUTPUT(stdout, "[SP DEBUG]:",                \
              msg, ##__VA_ARGS__)
			  
#define SP_PRINT(notif, msg, ...)					\
	SP_OUTPUT(stdout, notif, msg, ##__VA_ARGS__)	\
	
#else
#define SP_INFO(msg, ...) ;
#define SP_DEBUG(msg, ...) ;
#define SP_PRINT(msg, ...);
#endif //SP_NO_DEBUG

#endif // DEBUG_H_INCLUDED
