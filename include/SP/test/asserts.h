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

#define SP_TEST_ASSERT_ACTION(c, action, msg, ...)		\
	do													\
	{													\
		if(!(c))										\
		{												\
			sp_test_status = SP_TEST_FAIL;	  			\
			SP_PRINT("[SP-test]:", msg, ##__VA_ARGS__);	\
			action;										\
			return;										\
		}												\
	} while(0)
		
#define SP_TEST_ASSERT(c, msg, ...) SP_TEST_ASSERT_ACTION(c, ;, msg, ##__VA_ARGS__)

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#endif

#define SP_ASSERT_GENERIC_EQUAL_WITH_ACTION(expected, actual, action) SP_TEST_ASSERT_ACTION(likely((expected) == (actual)), action, "Assertion failed at line %d")
#define SP_ASSERT_BOOL_EQUAL_WITH_ACTION(expected, actual, action) SP_TEST_ASSERT_ACTION(likely((expected) == (actual)), action, "Assertion failed at line %d; expected: %s, actual: %s", __LINE__, (expected) ? "true" : "false", (actual) ? "true" : "false")
#define SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(expected, actual, action) SP_TEST_ASSERT_ACTION(likely((expected) == (actual)), action, "Assertion failed at line %d; expected: %lld, actual: %lld", __LINE__, (SPlong)(expected), (SPlong)(actual))
#define SP_ASSERT_DECIMAL_EQUAL_WITH_ACTION(expected, actual, action) SP_TEST_ASSERT_ACTION(likely(spTestDecimalEqual((expected), (actual))), action, "Assertion failed at line %d; expected: %f, actual: %f", __LINE__, (SPdouble)(expected), (SPdouble)(actual))
#define SP_ASSERT_STRING_EQUAL_WITH_ACTION(expected, actual, action) SP_TEST_ASSERT_ACTION(likely(spTestStringEqual((expected), (actual))), action, "Assertion failed at line %d; expected: \"%s\", actual: \"%s\"", __LINE__, (expected), (actual))
#define SP_ASSERT_STRING_ALMOST_EQUAL_WITH_ACTION(expected, actual, n, action) SP_TEST_ASSERT_ACTION(likely(spTestStringAlmostEqual((expected), (actual), (n))), action, "Assertion failed at line %d; expected: \"%s\", actual: \"%s\" (comparing up to %lld characters)",  __LINE__, (expected), (actual), (SPsize)(n))
#define SP_ASSERT_PTR_EQUAL_WITH_ACTION(expected, actual) SP_ASSERT_GENERIC_EQUAL_WITH_ACTION(expected, actual, action)
#define SP_ASSERT_EQUAL_WITH_ACTION(expected, actual, action) SP_ASSERT_GENERIC_EQUAL_WITH_ACTION(expected, actual, action)

#define SP_ASSERT_GENERIC_NOT_EQUAL_WITH_ACTION(expected, actual, action) SP_TEST_ASSERT_ACTION(likely((expected) != (actual)), action, "not-equal-Assertion failed")
#define SP_ASSERT_BOOL_NOT_EQUAL_WITH_ACTION(expected, actual) SP_TEST_ASSERT_ACTION(likely((expected) != (actual)), "Assertion failed at line %d; expected: not %s, actual: %s", __LINE__, (expected) ? "true" : "false", (actual) ? "true" : "false")
#define SP_ASSERT_INTEGER_NOT_EQUAL_WITH_ACTION(expected, actual, action) SP_TEST_ASSERT_ACTION(likely((expected) != (actual)), action, "Assertion failed at line %d; expected: not %i, actual: %i", __LINE__, (expected), (actual))
#define SP_ASSERT_DECIMAL_NOT_EQUAL_WITH_ACTION(expected, actual, action) SP_TEST_ASSERT_ACTION(likely(!spTestDecimalEqual((expected), (actual))), action, "Assertion failed at line %d; expected: not %f, actual: %f", __LINE__, (expected), (actual))
#define SP_ASSERT_STRING_NOT_EQUAL_WITH_ACTION(expected, actual, action) SP_TEST_ASSERT_ACTION(likely(!spTestStringEqual((expected), (actual))), action, "Assertion failed at line %d; expected: not \"%s\", actual: \"%s\"", __LINE__, (expected), (actual))
#define SP_ASSERT_STRING_ALMOST_NOT_EQUAL_WITH_ACTION(expected, actual, n, action) SP_TEST_ASSERT_ACTION(likely(!spTestStringAlmostEqual((expected), (actual), (n))), action, "Assertion failed at line %d; expected: not \"%s\", actual: \"%s\" (comparing up to %lld characters)",  __LINE__, (expected), (actual), (SPsize)(n))
#define SP_ASSERT_PTR_NOT_EQUAL_WITH_ACTION(expected, actual, action) SP_ASSERT_GENERIC_NOT_EQUAL_WITH_ACTION(expected, actual, action)
#define SP_ASSERT_NOT_EQUAL_WITH_ACTION(expected, actual, action) SP_ASSERT_GENERIC_NOT_EQUAL_WITH_ACTION(expected, actual, action)

#define SP_ASSERT_TRUE_WITH_ACTION(c, action) SP_TEST_ASSERT_ACTION(likely(c), action, "Assertion failed at line %d; expected true",  __LINE__);
#define SP_ASSERT_FALSE_WITH_ACTION(c, action) SP_TEST_ASSERT_ACTION(likely(!(c)), action, "Assertion failed at line %d; expected false",  __LINE__);

#define SP_ASSERT_NULL_WITH_ACTION(p, action) SP_TEST_ASSERT_ACTION((p) == NULL, action, "Assertion failed at line %d; expected NULL",  __LINE__);
#define SP_ASSERT_NOT_NULL_WITH_ACTION(p, action) SP_TEST_ASSERT_ACTION((p) != NULL, action, "Assertion failed at line %d; expected non-NULL",  __LINE__);

#define SP_ASSERT_INTEGER_GREATER_WITH_ACTION(a, b, action) SP_TEST_ASSERT_ACTION((a) > (b), action, "Assertion failed at line %d; expected a > b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_INTEGER_GREATER_EQUAL_WITH_ACTION(a, b, action) SP_TEST_ASSERT_ACTION((a) >= (b), action, "Assertion failed at line %d; expected a >= b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_INTEGER_LESS_WITH_ACTION(a, b, action) SP_TEST_ASSERT_ACTION((a) < (b), action, "Assertion failed at line %d; expected a < b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_INTEGER_LESS_EQUAL_WITH_ACTION(a, b, action) SP_TEST_ASSERT_ACTION((a) <= (b), action, "Assertion failed at line %d; expected a <= b for a = %d, b = %d", __LINE__, (a), (b))

#define SP_ASSERT_DECIMAL_GREATER_WITH_ACTION(a, b, action) SP_TEST_ASSERT_ACTION((a) > (b), action, "Assertion failed at line %d; expected a > b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_DECIMAL_GREATER_EQUAL_WITH_ACTION(a, b, action) SP_TEST_ASSERT_ACTION(((a) > (b)) || spTestDecimalEqual((a), (b)), action, "Assertion failed at line %d; expected a >= b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_DECIMAL_LESS_WITH_ACTION(a, b, action) SP_TEST_ASSERT_ACTION((a) < (b), action, "Assertion failed at line %d; expected a < b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_DECIMAL_LESS_EQUAL_WITH_ACTION(a, b, action) SP_TEST_ASSERT_ACTION(((a) < (b)) || spTestDecimalEqual((a), (b)), action, "Assertion failed at line %d; expected a <= b for a = %d, b = %d", __LINE__, (a), (b))
#define SP_ASSERT_DECIMAL_ALMOST_EQUAL_WITH_ACTION(a, b, n, action) SP_TEST_ASSERT_ACTION(spTestDecimalAlmostEqual((a), (b), (n)), action, "Assertion failed at line %d; expected: %f, actual: %f (comparing up to %lld places)", __LINE__, (a), (b), (n))
#define SP_ASSERT_DECIMAL_ALMOST_NOT_EQUAL_WITH_ACTION(a, b, n, action) SP_TEST_ASSERT_ACTION(!spTestDecimalAlmostEqual((a), (b), (n)), action, "Assertion failed at line %d; expected: not %f, actual: %f (comparing up to %lld places)", __LINE__, (a), (b), (n))


#define SP_ASSERT_GENERIC_EQUAL(expected, actual) SP_ASSERT_GENERIC_EQUAL_WITH_ACTION(expected, actual, ;)
#define SP_ASSERT_BOOL_EQUAL(expected, actual) SP_ASSERT_BOOL_EQUAL_WITH_ACTION(expected, actual, ;)
#define SP_ASSERT_INTEGER_EQUAL(expected, actual) SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(expected, actual, ;)
#define SP_ASSERT_DECIMAL_EQUAL(expected, actual) SP_ASSERT_DECIMAL_EQUAL_WITH_ACTION(expected, actual, ;)
#define SP_ASSERT_STRING_EQUAL(expected, actual) SP_ASSERT_STRING_EQUAL_WITH_ACTION(expected, actual, ;)
#define SP_ASSERT_STRING_ALMOST_EQUAL(expected, actual, n) SP_ASSERT_STRING_ALMOST_EQUAL_WITH_ACTION(expected, actual, n, ;)
#define SP_ASSERT_PTR_EQUAL(expected, actual) SP_ASSERT_PTR_EQUAL_WITH_ACTION(expected, actual, ;)
#define SP_ASSERT_EQUAL(expected, actual) SP_ASSERT_EQUAL_WITH_ACTION(expected, actual, ;)

#define SP_ASSERT_GENERIC_NOT_EQUAL(expected, actual) SP_ASSERT_GENERIC_NOT_EQUAL_WITH_ACTION(expected, actual, ;)
#define SP_ASSERT_BOOL_NOT_EQUAL(expected, actual) SP_ASSERT_BOOL_NOT_EQUAL_WITH_ACTION(expected, actual, ;)
#define SP_ASSERT_INTEGER_NOT_EQUAL(expected, actual) SP_ASSERT_INTEGER_NOT_EQUAL_WITH_ACTION(expected, actual, ;)
#define SP_ASSERT_DECIMAL_NOT_EQUAL(expected, actual) SP_ASSERT_DECIMAL_NOT_EQUAL_WITH_ACTION(expected, actual, ;)
#define SP_ASSERT_STRING_NOT_EQUAL(expected, actual) SP_ASSERT_STRING_NOT_EQUAL_WITH_ACTION(expected, actual, ;)
#define SP_ASSERT_STRING_ALMOST_NOT_EQUAL(expected, actual, n) SP_ASSERT_STRING_ALMOST_NOT_EQUAL_WITH_ACTION(expected, actual, n, ;)
#define SP_ASSERT_PTR_NOT_EQUAL(expected, actual) SP_ASSERT_PTR_NOT_EQUAL_WITH_ACTION(expected, actual, ;)
#define SP_ASSERT_NOT_EQUAL(expected, actual) SP_ASSERT_GENERIC_NOT_EQUAL(expected, actual, ;)

#define SP_ASSERT_TRUE(c) SP_ASSERT_TRUE_WITH_ACTION(c, ;)
#define SP_ASSERT_FALSE(c) SP_ASSERT_FALSE_WITH_ACTION(c, ;)

#define SP_ASSERT_NULL(p) SP_ASSERT_NULL_WITH_ACTION(p, ;)
#define SP_ASSERT_NOT_NULL(p) SP_ASSERT_NOT_NULL_WITH_ACTION(p, ;)

#define SP_ASSERT_INTEGER_GREATER(a, b) SP_ASSERT_GREATER_INTEGER_WITH_ACTION(a, b, ;)
#define SP_ASSERT_INTEGER_GREATER_EQUAL(a, b) SP_ASSERT_GREATER_EQUAL_INTEGER_WITH_ACTION(a, b, ;)
#define SP_ASSERT_INTEGER_LESS(a, b)  SP_ASSERT_INTEGER_LESS_WITH_ACTION(a, b, ;)
#define SP_ASSERT_INTEGER_LESS_EQUAL(a, b) SP_ASSERT_INTEGER_LESS_EQUAL_WITH_ACTION(a, b, ;)

#define SP_ASSERT_GREATER_DECIMAL(a, b) SP_ASSERT_GREATER_DECIMAL_WITH_ACTION(a, b, ;)
#define SP_ASSERT_GREATER_EQUAL_DECIMAL(a, b) SP_ASSERT_GREATER_EQUAL_DECIMAL_WITH_ACTION(a, b, ;)
#define SP_ASSERT_DECIMAL_LESS(a, b) SP_ASSERT_DECIMAL_LESS_WITH_ACTION(a, b, ;)
#define SP_ASSERT_LESS_EQUAL_DECIMAL(a, b) SP_ASSERT_LESS_EQUAL_DECIMAL(a, b, ;)
#define SP_ASSERT_ALMOST_EQUAL(a, b, n) SP_ASSERT_ALMOST_EQUAL_WITH_ACTION(a, b, n, ;)
#define SP_ASSERT_ALMOST_NOT_EQUAL(a, b, n) SP_ASSERT_ALMOST_NOT_EQUAL_WITH_ACTION(a, b, n, ;)
#endif
