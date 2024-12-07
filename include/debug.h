/****************************************************************************
 * Copyright (c) 2024 Hidegi
 *
 * This software is provided ‘as-is’, without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ****************************************************************************/
#ifndef MT3_DEBUG_H
#define MT3_DEBUG_H
#ifdef NDEBUG
#define SP_NO_DEBUG
#endif

#define SP_OUTPUT(stream, notif, msg, ...)		\
{							\
	fprintf((stream), "%-16s", (notif));		\
	fprintf((stream), msg, ##__VA_ARGS__);		\
	fprintf((stream), "\n");			\
}

#define SP_WARNING(msg, ...)				\
	SP_OUTPUT(stderr, "[SP WARNING]:",		\
			  msg, ##__VA_ARGS__)		\

#define SP_ASSERT(c, msg, ...)				\
if(!(c))						\
{							\
	SP_WARNING(msg, ##__VA_ARGS__)			\
	assert(c);					\
}

#ifndef SP_NO_DEBUG
#define SP_INFO(msg, ...)				\
	SP_OUTPUT(stdout, "[SP INFO]:",			\
			  msg, ##__VA_ARGS__)		\

#define SP_DEBUG(msg, ...)				\
	SP_OUTPUT(stdout, "[SP DEBUG]:",		\
			  msg, ##__VA_ARGS__)
			  
#define SP_PRINT(notif, msg, ...)			\
	SP_OUTPUT(stdout, notif, msg, ##__VA_ARGS__)	\
	
#else
#define SP_INFO(msg, ...) ;
#define SP_DEBUG(msg, ...) ;
#define SP_PRINT(msg, ...);
#endif //SP_NO_DEBUG

#endif
