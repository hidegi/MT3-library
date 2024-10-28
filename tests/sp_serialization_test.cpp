#include "SP/test/unit.h"
#include "SP/utils/io.h"
#include "SP/sparse/mot.h"

void test_serialization_trivial()
{
	MOT_tree* tree = motAllocTree("head");
	SP_ASSERT_NOT_NULL(tree);
	
	motAddInt(tree, "x", 1);
	motAddInt(tree, "y", 2);
	motAddInt(tree, "fjiaw", 300);
	motAddInt(tree, "motex", 220);
	motAddInt(tree, "value", 30);
	motAddInt(tree, "nmg", -14543);
	motAddShort(tree, "nmg0", -14543);
	motAddByte(tree, "nmg1", true);
	motAddByte(tree, "nmg2", false);
	motAddLong(tree, "nmg3", 495845);
	motAddDouble(tree, "fjackfink", 1.998E+58);
	motAddFloat(tree, "fjackfink2", 0.00001f);
	motAddString(tree, "hidegion", "hidegion");
	/*
	motAddInt(tree, "z", 3);
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