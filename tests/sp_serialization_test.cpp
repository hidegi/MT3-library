#include "SP/test/unit.h"
#include "SP/utils/io.h"
#include "SP/sparse/mot.h"

void test_serialization_trivial()
{
	MOT_tree* tree = motAllocTree("head");
	SP_ASSERT_NOT_NULL(tree);
	
	motAddInteger(tree, "x", 1);
	motAddInteger(tree, "y", 2);
	motAddInteger(tree, "fjiaw", 300);
	motAddInteger(tree, "motex", 220);
	motAddInteger(tree, "value", 30);
	motAddInteger(tree, "nmg", 99);
	/*
	motAddInteger(tree, "z", 3);
	*/
	motPrintTree(tree);
	motFreeTree(tree);
}
int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
	SP_TEST_ADD(test_serialization_trivial);

	spTestRunAll();
	spTestTerminate();
	return 0;
}