#ifndef SP_TEST_H
#define SP_TEST_H
#include "platform.h"
#include "types.h"
#include "asserts.h"

#ifndef SP_DEFAULT_PRECISION
#define SP_DEFAULT_PRECISION 15
#endif
#define SP_TEST_PASS 0
#define SP_TEST_FAIL 1
#define SP_TEST_ADD(action) spTestAdd(#action, action)
#define SP_TEST_INIT(c, v)								\
	if(!spTestInit((c), (v)))							\
    {													\
        SP_WARNING("Failed to initialize SP-test");		\
        return 1;										\
    }
	
#ifdef __cplusplus
extern "C" {
#endif

typedef void(*SPaction)();

SP_API SPbool spTestInit(SPint argc, SPchar** argv);
SP_API SPbool spTestAdd(const SPchar* name, SPaction action);
SP_API SPbool spTestDecimalEqual(SPdouble a, SPdouble b);
SP_API SPbool spTestDecimalEqual(SPdouble a, SPdouble b);
SP_API SPbool spTestRunAll();

SP_API SPbool spTestDecimalAlmostEqual(SPdouble a, SPdouble b, SPsize places);
SP_API SPbool spTestStringEqual(const SPchar* a, const SPchar* b);
SP_API SPbool spTestStringAlmostEqual(const SPchar* a, const SPchar* b, SPsize);

SP_API void spTestTerminate();

#ifdef __cplusplus
}
#endif
#endif