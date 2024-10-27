#ifndef SP_TEST_ASSERTS_H
#define SP_TEST_ASSERTS_H

#ifdef SP_COMPILER_GNUC
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(  (x), 0)
#else
#define likely(x)   (x)
#define unlikely(x) (x)
#endif
extern SP_API SPint sp_test_status;
#define SP_TEST_ASSERT(c, msg, ...)             		\
    do                                          		\
    {                                           		\
        if(!(c))                                		\
        {                                       		\
            sp_test_status = SP_TEST_FAIL;      		\
			SP_PRINT("[SP-test]:", msg, ##__VA_ARGS__);	\
            return;                             		\
        }                                       		\
    } while(0)

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#endif

#define SP_ASSERT_GENERIC_EQUAL(expected, actual) SP_TEST_ASSERT(likely((expected) == (actual)), "Assertion failed at line %d")
#define SP_ASSERT_BOOL_EQUAL(expected, actual) SP_TEST_ASSERT(likely((expected) == (actual)), "Assertion failed at line %d; expected: %s, actual: %s", __LINE__, (expected) ? "true" : "false", (actual) ? "true" : "false")
#define SP_ASSERT_INTEGER_EQUAL(expected, actual) SP_TEST_ASSERT(likely((expected) == (actual)), "Assertion failed at line %d; expected: %lld, actual: %lld", __LINE__, (SPlong)(expected), (SPlong)(actual))
#define SP_ASSERT_DECIMAL_EQUAL(expected, actual) SP_TEST_ASSERT(likely(spTestDecimalEqual((expected), (actual))), "Assertion failed at line %d; expected: %f, actual: %f", __LINE__, (SPdouble)(expected), (SPdouble)(actual))
#define SP_ASSERT_STRING_EQUAL(expected, actual) SP_TEST_ASSERT(likely(spTestStringEqual((expected), (actual))), "Assertion failed at line %d; expected: \"%s\", actual: \"%s\"", __LINE__, (expected), (actual))
#define SP_ASSERT_STRING_ALMOST_EQUAL(expected, actual, n) SP_TEST_ASSERT(likely(spTestStringAlmostEqual((expected), (actual), (n))), "Assertion failed at line %d; expected: \"%s\", actual: \"%s\" (comparing up to %lld characters)",  __LINE__, (expected), (actual), (SPsize)(n))
#define SP_ASSERT_PTR_EQUAL(expected, actual) SP_ASSERT_GENERIC_EQUAL(expected, actual)
#define SP_ASSERT_EQUAL(expected, actual) SP_ASSERT_GENERIC_EQUAL(expected, actual)

#define SP_ASSERT_GENERIC_NOT_EQUAL(expected, actual) SP_TEST_ASSERT(likely((expected) != (actual)), "not-equal-Assertion failed")
#define SP_ASSERT_BOOL_NOT_EQUAL(expected, actual) SP_TEST_ASSERT(likely((expected) != (actual)), "Assertion failed at line %d; expected: not %s, actual: %s", __LINE__, (expected) ? "true" : "false", (actual) ? "true" : "false")
#define SP_ASSERT_INTEGER_NOT_EQUAL(expected, actual) SP_TEST_ASSERT(likely((expected) != (actual)), "Assertion failed at line %d; expected: not %i, actual: %i", __LINE__, (expected), (actual))
#define SP_ASSERT_DECIMAL_NOT_EQUAL(expected, actual) SP_TEST_ASSERT(likely(!spTestDecimalEqual((expected), (actual))), "Assertion failed at line %d; expected: not %f, actual: %f", __LINE__, (expected), (actual))
#define SP_ASSERT_STRING_NOT_EQUAL(expected, actual) SP_TEST_ASSERT(likely(!spTestStringEqual((expected), (actual))), "Assertion failed at line %d; expected: not \"%s\", actual: \"%s\"", __LINE__, (expected), (actual))
#define SP_ASSERT_STRING_ALMOST_NOT_EQUAL(expected, actual, n) SP_TEST_ASSERT(likely(!spTestStringAlmostEqual((expected), (actual), (n))), "Assertion failed at line %d; expected: not \"%s\", actual: \"%s\" (comparing up to %lld characters)",  __LINE__, (expected), (actual), (SPsize)(n))
#define SP_ASSERT_PTR_NOT_EQUAL(expected, actual) SP_ASSERT_GENERIC_NOT_EQUAL(expected, actual)
#define SP_ASSERT_NOT_EQUAL(expected, actual) SP_ASSERT_GENERIC_NOT_EQUAL(expected, actual)

#define SP_ASSERT_TRUE(c) SP_TEST_ASSERT(likely(c), "Assertion failed at line %d; expected true",  __LINE__);
#define SP_ASSERT_FALSE(c) SP_TEST_ASSERT(likely(!(c)), "Assertion failed at line %d; expected false",  __LINE__);

#define SP_ASSERT_NULL(p) SP_TEST_ASSERT((p) == NULL, "Assertion failed at line %d; expected NULL",  __LINE__);
#define SP_ASSERT_NOT_NULL(p) SP_TEST_ASSERT((p) != NULL, "Assertion failed at line %d; expected non-NULL",  __LINE__);

#define SP_ASSERT_GREATER_INTEGER(a, b) SP_TEST_ASSERT((a) > (b), "Assertion failed at line %d; expected a > b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_GREATER_EQUAL_INTEGER(a, b) SP_TEST_ASSERT((a) >= (b), "Assertion failed at line %d; expected a >= b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_LESS_INTEGER(a, b) SP_TEST_ASSERT((a) < (b), "Assertion failed at line %d; expected a < b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_LESS_EQUAL_INTEGER(a, b) SP_TEST_ASSERT((a) <= (b), "Assertion failed at line %d; expected a <= b for a = %d, b = %d", __LINE__, (a), (b))

#define SP_ASSERT_GREATER_DECIMAL(a, b) SP_TEST_ASSERT((a) > (b), "Assertion failed at line %d; expected a > b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_GREATER_EQUAL_DECIMAL(a, b) SP_TEST_ASSERT(((a) > (b)) || spTestDecimalEqual((a), (b)), "Assertion failed at line %d; expected a >= b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_LESS_DECIMAL(a, b) SP_TEST_ASSERT((a) < (b), "Assertion failed at line %d; expected a < b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_LESS_EQUAL_DECIMAL(a, b) SP_TEST_ASSERT(((a) < (b)) || spTestDecimalEqual((a), (b)), "Assertion failed at line %d; expected a <= b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_ALMOST_EQUAL(a, b, n) SP_TEST_ASSERT(spTestDecimalAlmostEqual((a), (b), (n)), "Assertion failed at line %d; expected: %f, actual: %f (comparing up to %lld places)", __LINE__, (a), (b), (n))
#define SP_ASSERT_ALMOST_NOT_EQUAL(a, b, n) SP_TEST_ASSERT(!spTestDecimalAlmostEqual((a), (b), (n)), "Assertion failed at line %d; expected: not %f, actual: %f (comparing up to %lld places)", __LINE__, (a), (b), (n))

#endif
