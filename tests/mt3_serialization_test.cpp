#include "unit.h"
#include "mt3.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

void test_simple()
{
	MT3_node tree = mt3_AllocTree();
	mt3_InsertByte(&tree, "byte_1", 1);
	mt3_InsertShort(&tree, "short_1", -123);
	mt3_InsertInt(&tree, "int_1", 1234567);
	mt3_InsertLong(&tree, "long_1", 1234567485);
	mt3_InsertFloat(&tree, "float_1", 134.45f);
	mt3_InsertDouble(&tree, "double_1", 5423.d);
	mt3_InsertString(&tree, "string_1", "motex");
	mt3_InsertByte(&tree, "byte_2", 1);
	mt3_InsertShort(&tree, "short_2", -123);
	mt3_InsertInt(&tree, "int_2", 1234567);
	mt3_InsertLong(&tree, "long_2", 1234567485);
	mt3_InsertFloat(&tree, "float_2", 134.45f);
	mt3_InsertDouble(&tree, "double_2", 5423.d);
	mt3_InsertString(&tree, "string_2", "gaming");
	
	SPbyte byte_array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
	mt3_InsertByteList(&tree, "byte_array", 9, byte_array);
	SP_ASSERT_TRUE_WITH_ACTION(mt3_VerifyRBT(tree), mt3_Delete(&tree));
	mt3_PrintTree(tree);
	mt3_Delete(&tree);
}

int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
	
	SP_TEST_ADD(test_simple);
	spTestRunAll();
	spTestTerminate();
	return 0;
}