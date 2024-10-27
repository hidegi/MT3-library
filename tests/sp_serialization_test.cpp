#include "SP/test/unit.h"
#include "SP/utils/io.h"
#include "SP/sparse/mot.h"

void test_serialization_trivial()
{
	/*
	long long h1 = sdbmHashGPT("motex");
	long long h2 = sdbmHashGithub("motex");
	
	SP_ASSERT_INTEGER_EQUAL(h1, h2);
	SP_DEBUG("hash: %lld", h1);
	*/
}
int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
	SP_TEST_ADD(test_serialization_trivial);

	spTestRunAll();
	spTestTerminate();
	return 0;
}