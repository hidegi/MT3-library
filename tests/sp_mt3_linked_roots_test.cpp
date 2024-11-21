#include "SP/test/unit.h"
#include "SP/utils/io.h"
#include "SP/sparse/mt3.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

static void sp_test_root_array_from_null()
{
	MT3_tree tree1 = NULL;
	MT3_tree tree2 = NULL;
	MT3_tree tree3 = NULL;
	MT3_tree tree4 = NULL;
	
	mt3_InsertString(&tree1, "a", "sub_a");
    mt3_InsertString(&tree1, "b", "sub_b");
    mt3_InsertString(&tree1, "c", "sub_c");
    mt3_InsertString(&tree1, "d", "sub_d");
	
	mt3_InsertString(&tree2, "e", "sub_e");
    mt3_InsertString(&tree2, "f", "sub_f");
    mt3_InsertString(&tree2, "g", "sub_g");
    mt3_InsertString(&tree2, "h", "sub_h");
	
	mt3_InsertString(&tree3, "i", "sub_i");
    mt3_InsertString(&tree3, "j", "sub_j");
    mt3_InsertString(&tree3, "k", "sub_k");
    mt3_InsertString(&tree3, "l", "sub_l");
	
	mt3_InsertString(&tree4, "m", "sub_m");
    mt3_InsertString(&tree4, "n", "sub_n");
    mt3_InsertString(&tree4, "o", "sub_o");
    mt3_InsertString(&tree4, "p", "sub_p");
	
	MT3_array list = NULL;
	MT3_tree head = NULL;
	mt3_ArrayInsertTree(&list, tree1);
	mt3_ArrayInsertTree(&list, tree2);
	
	mt3_ArrayInsertTree(&list, tree3);
	mt3_ArrayInsertTree(&list, tree4);
	mt3_InsertString(&head, "1", "hidegi");
    mt3_InsertString(&head, "2", "motex");
    mt3_InsertString(&head, "3", "betelgus");
    mt3_InsertString(&head, "4", "fjiaw");
	mt3_InsertArray(&head, "tree_list", list);
	
	
	SPbuffer buffer = mt3_WriteBinary(head);
	
    MT3_tree output = mt3_ReadBinary(buffer);
	
	
    printf("expected:\n");
    mt3_PrintTree(head);

    printf("actual:\n");
    mt3_PrintTree(output);
    printf("(%lld bytes)\n\n", buffer.length);
	
	mt3_FreeArray(&list);
	mt3_FreeTree(&head);
	mt3_FreeTree(&output);
	spBufferFree(&buffer);
}

static void sp_test_root_array_from_alloc()
{
	MT3_tree tree1 = NULL;
	MT3_tree tree2 = NULL;
	MT3_tree tree3 = NULL;
	MT3_tree tree4 = NULL;
	
	mt3_InsertString(&tree1, "a", "sub_a");
    mt3_InsertString(&tree1, "b", "sub_b");
    mt3_InsertString(&tree1, "c", "sub_c");
    mt3_InsertString(&tree1, "d", "sub_d");
	
	mt3_InsertString(&tree2, "e", "sub_e");
    mt3_InsertString(&tree2, "f", "sub_f");
    mt3_InsertString(&tree2, "g", "sub_g");
    mt3_InsertString(&tree2, "h", "sub_h");
	
	mt3_InsertString(&tree3, "i", "sub_i");
    mt3_InsertString(&tree3, "j", "sub_j");
    mt3_InsertString(&tree3, "k", "sub_k");
    mt3_InsertString(&tree3, "l", "sub_l");
	
	mt3_InsertString(&tree4, "m", "sub_m");
    mt3_InsertString(&tree4, "n", "sub_n");
    mt3_InsertString(&tree4, "o", "sub_o");
    mt3_InsertString(&tree4, "p", "sub_p");
	
	MT3_array list = mt3_AllocArray();
	MT3_tree head = NULL;
	mt3_ArrayInsertTree(&list, tree1);
	mt3_ArrayInsertTree(&list, tree2);
	
	mt3_ArrayInsertTree(&list, tree3);
	mt3_ArrayInsertTree(&list, tree4);
	mt3_InsertString(&head, "1", "hidegi");
    mt3_InsertString(&head, "2", "motex");
    mt3_InsertString(&head, "3", "betelgus");
    mt3_InsertString(&head, "4", "fjiaw");
	mt3_InsertArray(&head, "tree_list", list);
	
	
	SPbuffer buffer = mt3_WriteBinary(head);
	
    MT3_tree output = mt3_ReadBinary(buffer);
	
	
    printf("expected:\n");
    mt3_PrintTree(head);

    printf("actual:\n");
    mt3_PrintTree(output);
    printf("(%lld bytes)\n\n", buffer.length);
	
	mt3_FreeArray(&list);
	mt3_FreeTree(&head);
	mt3_FreeTree(&output);
	spBufferFree(&buffer);
}

static void test_byte_array_from_null()
{
	SPbyte dataset1[] = {1, 2, 3, 4, 5, 6, 7, 8};
	SPbyte dataset2[] = {8, 7, 6, 5, 4, 3, 2, 1};
	SPbyte dataset3[] = {120, 7, 31, 93, 29};
	MT3_array array = NULL;
	mt3_ArrayInsertByteArray(&array, sizeof(dataset1) / sizeof(SPbyte), dataset1);
	mt3_ArrayInsertByteArray(&array, sizeof(dataset2) / sizeof(SPbyte), dataset2);
	mt3_ArrayInsertByteArray(&array, sizeof(dataset3) / sizeof(SPbyte), dataset3);
	
	MT3_tree head = NULL;
	mt3_InsertArray(&head, "byte_array", array);
	mt3_PrintTree(head);
	mt3_FreeTree(&head);
	mt3_FreeArray(&array);
}

int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
	//SP_TEST_ADD(sp_test_root_array_from_null);
	SP_TEST_ADD(sp_test_root_array_from_alloc);
	SP_TEST_ADD(test_byte_array_from_null);

	spTestRunAll();
	spTestTerminate();
	return 0;
}