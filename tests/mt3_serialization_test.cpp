#include "unit.h"
#include "mt3.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

void test_simple()
{
    MT3_node subtree = NULL;
    mt3_InsertString(&subtree, "str1", "motex");
    mt3_InsertString(&subtree, "str2", "gaming");
    mt3_InsertString(&subtree, "str3", "is");
    mt3_InsertString(&subtree, "str4", "ugly");
    SP_ASSERT_TRUE_WITH_ACTION(mt3_IsTree(subtree), mt3_Delete(&subtree));

    MT3_node list = NULL;
    mt3_Append(&list, subtree);
    mt3_Append(&list, subtree);
    mt3_Append(&list, subtree);
    mt3_Append(&list, subtree);

	MT3_node tree = NULL;

	mt3_InsertByte(&tree, "byte_1", 1);
	mt3_InsertShort(&tree, "short_1", -123);
	mt3_InsertInt(&tree, "int_1", 1234567);
	mt3_InsertLong(&tree, "long_1", 1234567485);
	mt3_InsertFloat(&tree, "float_1", 134.45f);
	mt3_InsertDouble(&tree, "double_1", 5423);
	mt3_InsertString(&tree, "string_1", "motex");
	mt3_InsertByte(&tree, "byte_2", 1);
	mt3_InsertShort(&tree, "short_2", -123);
	mt3_InsertInt(&tree, "int_2", 1234567);
	mt3_InsertLong(&tree, "long_2", 1234567485);
	mt3_InsertFloat(&tree, "float_2", 134.45f);
	mt3_InsertDouble(&tree, "double_2", 5423);
	mt3_InsertString(&tree, "string_2", "gaming");
	mt3_Insert(&tree, "subtree", subtree);
	mt3_Insert(&tree, "list", list);

	MT3_node multi_list = NULL;
	mt3_Append(&multi_list, list);
	mt3_Append(&multi_list, list);
	mt3_Append(&multi_list, list);

	SPbyte byte_array[] = {1, 2, 3, 4, 5, -6, -7, -8, -9};
	SPshort short_array[] = {1, 2, 3, 4, 5, -6, -7, -8, -9};
	SPint int_array[] = {1, 2, 3, 4, 5, -6, -7, -8, -9};
	SPlong long_array[] = {1, 2, 3, 4, 5, -6, -7, -8, -9};
	SPfloat float_array[] = {1.3f, 2.4f, 5.4f, 252.f, 19.f, 43.f, 74.f};
	SPdouble double_array[] = {1.3, 2.4, 5.4, 252.0, 19.0, 43.0, 74.0};
    const SPchar* string_array[] = {"hda1666", "sp1667", "fjiaw", "betel"};

	mt3_InsertByteList(&tree, "byte_array", 9, byte_array);
	mt3_InsertShortList(&tree, "short_aray", 9, short_array);
	mt3_InsertIntList(&tree, "int_aray", 9, int_array);
	mt3_InsertLongList(&tree, "long_aray", 9, long_array);
	mt3_InsertFloatList(&tree, "float_aray", sizeof(float_array) / sizeof(SPfloat), float_array);
	mt3_InsertDoubleList(&tree, "double_aray", sizeof(double_array) / sizeof(SPdouble), double_array);
	mt3_InsertStringList(&tree, "string_array", sizeof(string_array) / sizeof(SPchar*), string_array);
	mt3_Insert(&tree, "multi_list", multi_list);

	SP_ASSERT_TRUE_WITH_ACTION(mt3_VerifyRBT(tree), mt3_Delete(&tree));
	mt3_PrintTree(tree);
	mt3_Delete(&tree);
	mt3_Delete(&subtree);
	mt3_Delete(&list);
}

int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
	
	SP_TEST_ADD(test_simple);
	spTestRunAll();
	spTestTerminate();
	return 0;
}