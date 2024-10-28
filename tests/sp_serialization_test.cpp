#include "SP/test/unit.h"
#include "SP/utils/io.h"
#include "SP/sparse/mot.h"

void test_serialization_trivial()
{
	MOT_tree* tree = motAllocTree("head");
	SP_ASSERT_NOT_NULL(tree);
	
	motInsertInt(tree, "x", 1);
	motInsertInt(tree, "y", 2);
	motInsertInt(tree, "fjiaw", 300);
	motInsertInt(tree, "motex", 220);
	motInsertInt(tree, "value", 30);
	motInsertInt(tree, "nmg", -14543);
	motInsertShort(tree, "nmg0", -14543);
	motInsertByte(tree, "nmg1", true);
	motInsertByte(tree, "nmg2", false);
	motInsertLong(tree, "nmg3", 495845);
	motInsertDouble(tree, "fjackfink", 1.998E+58);
	motInsertFloat(tree, "fjackfink2", 0.00001f);
	motInsertString(tree, "hidegion", "hidegion");
	/*
	motInsertInt(tree, "z", 3);
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