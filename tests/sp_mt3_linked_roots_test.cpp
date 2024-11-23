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

static void test_byte_array_from_alloc()
{
	SPbyte dataset1[] = {1, 2, 3, 4, 5, 6, 7, 8};
	SPbyte dataset2[] = {8, 7, 6, 5, 4, 3, 2, 1};
	SPbyte dataset3[] = {120, 7, 31, 93, 29};
	MT3_array array = mt3_AllocArray();
	mt3_ArrayInsertByteArray(&array, sizeof(dataset1) / sizeof(SPbyte), dataset1);
	mt3_ArrayInsertByteArray(&array, sizeof(dataset2) / sizeof(SPbyte), dataset2);
	mt3_ArrayInsertByteArray(&array, sizeof(dataset3) / sizeof(SPbyte), dataset3);
	
	MT3_tree head = NULL;
	mt3_InsertArray(&head, "byte_array", array);
	mt3_PrintTree(head);
	mt3_FreeTree(&head);
	mt3_FreeArray(&array);
}

static void test_byte_array_read_write()
{
	SPbyte dataset1[] = {1, 2, 3, 4, 5, 6, 7, 8};
	SPbyte dataset2[] = {8, 7, 6, 5, 4, 3, 2, 1};
	SPbyte dataset3[] = {120, 7, 31, 93, 29};
	MT3_array array = mt3_AllocArray();
	mt3_ArrayInsertByteArray(&array, sizeof(dataset1) / sizeof(SPbyte), dataset1);
	mt3_ArrayInsertByteArray(&array, sizeof(dataset2) / sizeof(SPbyte), dataset2);
	mt3_ArrayInsertByteArray(&array, sizeof(dataset3) / sizeof(SPbyte), dataset3);
	
	MT3_tree head = NULL;
	mt3_InsertArray(&head, "byte_array", array);
	
	SPbuffer buffer = mt3_WriteBinary(head);
    MT3_tree output = mt3_ReadBinary(buffer);
	
    printf("expected:\n");
    mt3_PrintTree(head);

    printf("actual:\n");
    mt3_PrintTree(output);
	
	mt3_FreeTree(&head);
	mt3_FreeArray(&array);
	mt3_FreeTree(&output);
	spBufferFree(&buffer);
}

static void test_array_of_arrays()
{
	SPbyte dataset1[] = {-76, -17, 33, 109, -3, 42, -42, 62, 116, 34};
	SPbyte dataset2[] = {-39, 47, -59, 4, 97, 59, 8, 11, -90, -10};
	SPbyte dataset3[] = {-41, -79, -52, 61, 98, -66, 81, 75, -100, 113};
	
	SPbyte dataset4[] = {-115, -76, 113, 57, 120, 59, 52, -89, 51, -118};
	SPbyte dataset5[] = {69, 18, -94, -50, -43, -28, -108, -7, -106, 40};
	SPbyte dataset6[] = {11, -43, -92, -52, -65, -27, 109, 99, 6, 91};
	
	SPbyte dataset7[] = {52, -65, -90, 80, 54, -22, 117, 90, -8, -92};
	SPbyte dataset8[] = {-96, -11, -103, 42, -78, -71, -59, -90, 21, -47};
	SPbyte dataset9[] = {119, -31, -73, 90, 31, -80, 86, -100, 106, 13};
	
	MT3_array array1 = NULL;
	MT3_array array2 = NULL;
	MT3_array array3 = NULL;
	MT3_array array_arrays = NULL;
	
	mt3_ArrayInsertByteArray(&array1, sizeof(dataset1) / sizeof(SPbyte), dataset1);
	mt3_ArrayInsertByteArray(&array1, sizeof(dataset2) / sizeof(SPbyte), dataset2);
	mt3_ArrayInsertByteArray(&array1, sizeof(dataset3) / sizeof(SPbyte), dataset3);
	
	mt3_ArrayInsertByteArray(&array2, sizeof(dataset4) / sizeof(SPbyte), dataset4);
	mt3_ArrayInsertByteArray(&array2, sizeof(dataset5) / sizeof(SPbyte), dataset5);
	mt3_ArrayInsertByteArray(&array2, sizeof(dataset6) / sizeof(SPbyte), dataset6);
	
	mt3_ArrayInsertByteArray(&array3, sizeof(dataset7) / sizeof(SPbyte), dataset7);
	mt3_ArrayInsertByteArray(&array3, sizeof(dataset8) / sizeof(SPbyte), dataset8);
	mt3_ArrayInsertByteArray(&array3, sizeof(dataset9) / sizeof(SPbyte), dataset9);
	
	mt3_ArrayInsertArray(&array_arrays, array1);
	mt3_ArrayInsertArray(&array_arrays, array2);
	mt3_ArrayInsertArray(&array_arrays, array3);
	
	MT3_tree head = NULL;
	mt3_InsertArray(&head, "multi_array", array_arrays);
	mt3_PrintTree(head);
	
	mt3_FreeTree(&head);
	mt3_FreeArray(&array_arrays);
	mt3_FreeArray(&array1);
	mt3_FreeArray(&array2);
	mt3_FreeArray(&array3);
}

static void test_array_of_arrays_read_write_bytes()
{
	SPbyte dataset1[] = {-76, -17, 33, 109, -3, 42, -42, 62, 116, 34};
	SPbyte dataset2[] = {-39, 47, -59, 4, 97, 59, 8, 11, -90, -10};
	SPbyte dataset3[] = {-41, -79, -52, 61, 98, -66, 81, 75, -100, 113};
	
	SPbyte dataset4[] = {-115, -76, 113, 57, 120, 59, 52, -89, 51, -118};
	SPbyte dataset5[] = {69, 18, -94, -50, -43, -28, -108, -7, -106, 40};
	SPbyte dataset6[] = {11, -43, -92, -52, -65, -27, 109, 99, 6, 91};
	
	SPbyte dataset7[] = {52, -65, -90, 80, 54, -22, 117, 90, -8, -92};
	SPbyte dataset8[] = {-96, -11, -103, 42, -78, -71, -59, -90, 21, -47};
	SPbyte dataset9[] = {119, -31, -73, 90, 31, -80, 86, -100, 106, 13};
	
	MT3_array array1 = NULL;
	MT3_array array2 = NULL;
	MT3_array array3 = NULL;
	MT3_array array_arrays = NULL;
	
	mt3_ArrayInsertByteArray(&array1, sizeof(dataset1) / sizeof(SPbyte), dataset1);
	mt3_ArrayInsertByteArray(&array1, sizeof(dataset2) / sizeof(SPbyte), dataset2);
	mt3_ArrayInsertByteArray(&array1, sizeof(dataset3) / sizeof(SPbyte), dataset3);
	
	mt3_ArrayInsertByteArray(&array2, sizeof(dataset4) / sizeof(SPbyte), dataset4);
	mt3_ArrayInsertByteArray(&array2, sizeof(dataset5) / sizeof(SPbyte), dataset5);
	mt3_ArrayInsertByteArray(&array2, sizeof(dataset6) / sizeof(SPbyte), dataset6);
	
	mt3_ArrayInsertByteArray(&array3, sizeof(dataset7) / sizeof(SPbyte), dataset7);
	mt3_ArrayInsertByteArray(&array3, sizeof(dataset8) / sizeof(SPbyte), dataset8);
	mt3_ArrayInsertByteArray(&array3, sizeof(dataset9) / sizeof(SPbyte), dataset9);
	
	mt3_ArrayInsertArray(&array_arrays, array1);
	mt3_ArrayInsertArray(&array_arrays, array2);
	mt3_ArrayInsertArray(&array_arrays, array3);
	
	MT3_tree head = NULL;
	mt3_InsertArray(&head, "multi_array", array_arrays);
	
	SPbuffer buffer = mt3_WriteBinary(head);
	
    MT3_tree output = mt3_ReadBinary(buffer);
	
    printf("expected:\n");
    mt3_PrintTree(head);

    printf("actual:\n");
    mt3_PrintTree(output);
	
	mt3_FreeTree(&head);
	mt3_FreeArray(&array_arrays);
	mt3_FreeArray(&array1);
	mt3_FreeArray(&array2);
	mt3_FreeArray(&array3);
	mt3_FreeTree(&output);
	spBufferFree(&buffer);
}

static void test_array_of_arrays_read_write_shorts()
{
	SPshort dataset1[] = {-76, -17, 33, 109, -3, 42, -42, 62, 116, 34};
	SPshort dataset2[] = {-39, 47, -59, 4, 97, 59, 8, 11, -90, -10};
	SPshort dataset3[] = {-41, -79, -52, 61, 98, -66, 81, 75, -100, 113};
	
	SPshort dataset4[] = {-115, -76, 113, 57, 120, 59, 52, -89, 51, -118};
	SPshort dataset5[] = {69, 18, -94, -50, -43, -28, -108, -7, -106, 40};
	SPshort dataset6[] = {11, -43, -92, -52, -65, -27, 109, 99, 6, 91};
	
	SPshort dataset7[] = {52, -65, -90, 80, 54, -22, 117, 90, -8, -92};
	SPshort dataset8[] = {-96, -11, -103, 42, -78, -71, -59, -90, 21, -47};
	SPshort dataset9[] = {119, -31, -73, 90, 31, -80, 86, -100, 106, 13};
	
	MT3_array array1 = NULL;
	MT3_array array2 = NULL;
	MT3_array array3 = NULL;
	MT3_array array_arrays = NULL;
	
	mt3_ArrayInsertShortArray(&array1, sizeof(dataset1) / sizeof(SPshort), dataset1);
	mt3_ArrayInsertShortArray(&array1, sizeof(dataset2) / sizeof(SPshort), dataset2);
	mt3_ArrayInsertShortArray(&array1, sizeof(dataset3) / sizeof(SPshort), dataset3);
	
	mt3_ArrayInsertShortArray(&array2, sizeof(dataset4) / sizeof(SPshort), dataset4);
	mt3_ArrayInsertShortArray(&array2, sizeof(dataset5) / sizeof(SPshort), dataset5);
	mt3_ArrayInsertShortArray(&array2, sizeof(dataset6) / sizeof(SPshort), dataset6);
	
	mt3_ArrayInsertShortArray(&array3, sizeof(dataset7) / sizeof(SPshort), dataset7);
	mt3_ArrayInsertShortArray(&array3, sizeof(dataset8) / sizeof(SPshort), dataset8);
	mt3_ArrayInsertShortArray(&array3, sizeof(dataset9) / sizeof(SPshort), dataset9);
	
	mt3_ArrayInsertArray(&array_arrays, array1);
	mt3_ArrayInsertArray(&array_arrays, array2);
	mt3_ArrayInsertArray(&array_arrays, array3);
	
	MT3_tree head = NULL;
	mt3_InsertArray(&head, "multi_array", array_arrays);
	mt3_InsertByte(&head, "byte1", -20);
	mt3_InsertShort(&head, "short1", -20);
	mt3_InsertInt(&head, "int1", -20);
	mt3_InsertLong(&head, "long1", -20);
	mt3_InsertLong(&head, "long2", -21);
	mt3_Delete(&head, "multi_array"); 
	SPbuffer buffer = mt3_WriteBinary(head);
	
    MT3_tree output = mt3_ReadBinary(buffer);
	
    printf("expected:\n");
    mt3_PrintTree(head);

    printf("actual:\n");
    mt3_PrintTree(output);
	
	printf("(%lld bytes)\n", buffer.length);
	mt3_FreeTree(&head);
	mt3_FreeArray(&array_arrays);
	mt3_FreeArray(&array1);
	mt3_FreeArray(&array2);
	mt3_FreeArray(&array3);
	mt3_FreeTree(&output);
	spBufferFree(&buffer);
}

static void test_array_of_arrays_read_write_bytes_ints()
{
	SPint dataset1[] = {-76, -17, 33, 109, -3, 42, -42, 62, 116, 34};
	SPint dataset2[] = {-39, 47, -59, 4, 97, 59, 8, 11, -90, -10};
	SPint dataset3[] = {-41, -79, -52, 61, 98, -66, 81, 75, -100, 113};
	
	SPint dataset4[] = {-115, -76, 113, 57, 120, 59, 52, -89, 51, -118};
	SPint dataset5[] = {69, 18, -94, -50, -43, -28, -108, -7, -106, 40};
	SPint dataset6[] = {11, -43, -92, -52, -65, -27, 109, 99, 6, 91};
	
	SPint dataset7[] = {52, -65, -90, 80, 54, -22, 117, 90, -8, -92};
	SPint dataset8[] = {-96, -11, -103, 42, -78, -71, -59, -90, 21, -47};
	SPint dataset9[] = {119, -31, -73, 90, 31, -80, 86, -100, 106, 13};
	
	MT3_array array1 = NULL;
	MT3_array array2 = NULL;
	MT3_array array3 = NULL;
	MT3_array array_arrays = NULL;
	
	mt3_ArrayInsertIntArray(&array1, sizeof(dataset1) / sizeof(SPint), dataset1);
	mt3_ArrayInsertIntArray(&array1, sizeof(dataset2) / sizeof(SPint), dataset2);
	mt3_ArrayInsertIntArray(&array1, sizeof(dataset3) / sizeof(SPint), dataset3);
	
	mt3_ArrayInsertIntArray(&array2, sizeof(dataset4) / sizeof(SPint), dataset4);
	mt3_ArrayInsertIntArray(&array2, sizeof(dataset5) / sizeof(SPint), dataset5);
	mt3_ArrayInsertIntArray(&array2, sizeof(dataset6) / sizeof(SPint), dataset6);
	
	mt3_ArrayInsertIntArray(&array3, sizeof(dataset7) / sizeof(SPint), dataset7);
	mt3_ArrayInsertIntArray(&array3, sizeof(dataset8) / sizeof(SPint), dataset8);
	mt3_ArrayInsertIntArray(&array3, sizeof(dataset9) / sizeof(SPint), dataset9);
	
	mt3_ArrayInsertArray(&array_arrays, array1);
	mt3_ArrayInsertArray(&array_arrays, array2);
	mt3_ArrayInsertArray(&array_arrays, array3);
	
	MT3_tree head = NULL;
	mt3_InsertArray(&head, "multi_array", array_arrays);
	
	SPbuffer buffer = mt3_WriteBinary(head);
	
    MT3_tree output = mt3_ReadBinary(buffer);
	
    printf("expected:\n");
    mt3_PrintTree(head);

    printf("actual:\n");
    mt3_PrintTree(output);
	
	printf("(%lld bytes)\n", buffer.length);
	mt3_FreeTree(&head);
	mt3_FreeArray(&array_arrays);
	mt3_FreeArray(&array1);
	mt3_FreeArray(&array2);
	mt3_FreeArray(&array3);
	mt3_FreeTree(&output);
	spBufferFree(&buffer);
}

static void test_array_of_arrays_read_write_longs()
{
	SPlong dataset1[] = {-76, -17, 33, 109, -3, 42, -42, 62, 116, 34};
	SPlong dataset2[] = {-39, 47, -59, 4, 97, 59, 8, 11, -90, -10};
	SPlong dataset3[] = {-41, -79, -52, 61, 98, -66, 81, 75, -100, 113};
	
	SPlong dataset4[] = {-115, -76, 113, 57, 120, 59, 52, -89, 51, -118};
	SPlong dataset5[] = {69, 18, -94, -50, -43, -28, -108, -7, -106, 40};
	SPlong dataset6[] = {11, -43, -92, -52, -65, -27, 109, 99, 6, 91};
	
	SPlong dataset7[] = {52, -65, -90, 80, 54, -22, 117, 90, -8, -92};
	SPlong dataset8[] = {-96, -11, -103, 42, -78, -71, -59, -90, 21, -47};
	SPlong dataset9[] = {119, -31, -73, 90, 31, -80, 86, -100, 106, 13};
	
	MT3_array array1 = NULL;
	MT3_array array2 = NULL;
	MT3_array array3 = NULL;
	MT3_array array_arrays = NULL;
	
	mt3_ArrayInsertLongArray(&array1, sizeof(dataset1) / sizeof(SPlong), dataset1);
	mt3_ArrayInsertLongArray(&array1, sizeof(dataset2) / sizeof(SPlong), dataset2);
	mt3_ArrayInsertLongArray(&array1, sizeof(dataset3) / sizeof(SPlong), dataset3);
	
	mt3_ArrayInsertLongArray(&array2, sizeof(dataset4) / sizeof(SPlong), dataset4);
	mt3_ArrayInsertLongArray(&array2, sizeof(dataset5) / sizeof(SPlong), dataset5);
	mt3_ArrayInsertLongArray(&array2, sizeof(dataset6) / sizeof(SPlong), dataset6);
	
	mt3_ArrayInsertLongArray(&array3, sizeof(dataset7) / sizeof(SPlong), dataset7);
	mt3_ArrayInsertLongArray(&array3, sizeof(dataset8) / sizeof(SPlong), dataset8);
	mt3_ArrayInsertLongArray(&array3, sizeof(dataset9) / sizeof(SPlong), dataset9);
	
	mt3_ArrayInsertArray(&array_arrays, array1);
	mt3_ArrayInsertArray(&array_arrays, array2);
	mt3_ArrayInsertArray(&array_arrays, array3);
	
	MT3_tree head = NULL;
	mt3_InsertArray(&head, "multi_array", array_arrays);
	
	SPbuffer buffer = mt3_WriteBinary(head);
	
    MT3_tree output = mt3_ReadBinary(buffer);
	
    printf("expected:\n");
    mt3_PrintTree(head);

    printf("actual:\n");
    mt3_PrintTree(output);
	
	printf("(%lld bytes)\n", buffer.length);
	mt3_FreeTree(&head);
	mt3_FreeArray(&array_arrays);
	mt3_FreeArray(&array1);
	mt3_FreeArray(&array2);
	mt3_FreeArray(&array3);
	mt3_FreeTree(&output);
	spBufferFree(&buffer);
}

static void test_array_of_arrays_read_write_bytes_strings()
{
	const SPchar* dataset1[] = {"a", "b", "c"};
	const SPchar* dataset2[] = {"e", "f", "g"};
	const SPchar* dataset3[] = {"g", "h", "i"};
	
	const SPchar* dataset4[] = {"j", "k", "l"};
	const SPchar* dataset5[] = {"m", "n", "o"};
	const SPchar* dataset6[] = {"p", "q", "r"};
	
	const SPchar* dataset7[] = {"s", "t", "u"};
	const SPchar* dataset8[] = {"v", "w", "x"};
	const SPchar* dataset9[] = {"y", "z"};
	const SPchar* dataset10[] = {"motex", "hidegi"};
	
	MT3_array array1 = NULL;
	MT3_array array2 = NULL;
	MT3_array array3 = NULL;
	MT3_array array_arrays = NULL;
	
	mt3_ArrayInsertStringArray(&array1, sizeof(dataset1) / sizeof(const SPchar*), dataset1);
	mt3_ArrayInsertStringArray(&array1, sizeof(dataset2) / sizeof(const SPchar*), dataset2);
	mt3_ArrayInsertStringArray(&array1, sizeof(dataset3) / sizeof(const SPchar*), dataset3);
	
	mt3_ArrayInsertStringArray(&array2, sizeof(dataset4) / sizeof(const SPchar*), dataset4);
	mt3_ArrayInsertStringArray(&array2, sizeof(dataset5) / sizeof(const SPchar*), dataset5);
	mt3_ArrayInsertStringArray(&array2, sizeof(dataset6) / sizeof(const SPchar*), dataset6);
	
	mt3_ArrayInsertStringArray(&array3, sizeof(dataset7) / sizeof(const SPchar*), dataset7);
	mt3_ArrayInsertStringArray(&array3, sizeof(dataset8) / sizeof(const SPchar*), dataset8);
	mt3_ArrayInsertStringArray(&array3, sizeof(dataset9) / sizeof(const SPchar*), dataset9);
	mt3_ArrayInsertStringArray(&array3, sizeof(dataset10) / sizeof(const SPchar*), dataset10);
	
	mt3_ArrayInsertArray(&array_arrays, array1);
	mt3_ArrayInsertArray(&array_arrays, array2);
	mt3_ArrayInsertArray(&array_arrays, array3);
	
	MT3_tree head = NULL;
	mt3_InsertArray(&head, "multi_array", array_arrays);
	
	SPbuffer buffer = mt3_WriteBinary(head);
	
    MT3_tree output = mt3_ReadBinary(buffer);
	
    printf("expected:\n");
    mt3_PrintTree(head);

    printf("actual:\n");
    mt3_PrintTree(output);
	
	mt3_FreeTree(&head);
	mt3_FreeArray(&array_arrays);
	mt3_FreeArray(&array1);
	mt3_FreeArray(&array2);
	mt3_FreeArray(&array3);
	mt3_FreeTree(&output);
	spBufferFree(&buffer);
}

static void test_array_of_arrays_read_write_bytes_floats()
{
	float dataset1[] = {1.4, 2.43, 954.23, 1.32, 69.423};
	float dataset2[] = {96.8945f, 965.f};
	float dataset3[] = {56.35435f, .5652f, 182.3E+12f, 45.34};
	float dataset4[] = {86.43, 1.4171f, .866, .5838, 182.34f};
	float dataset5[] = {6878.34, 8128, .344, 969.23, 7.423, 10.444};
	
	MT3_array array1 = NULL;
	MT3_array array2 = NULL;
	MT3_array array_arrays = NULL;
	
	mt3_ArrayInsertFloatArray(&array1, sizeof(dataset1) / sizeof(float), dataset1);
	mt3_ArrayInsertFloatArray(&array1, sizeof(dataset2) / sizeof(float), dataset2);
	mt3_ArrayInsertFloatArray(&array1, sizeof(dataset3) / sizeof(float), dataset3);
	
	mt3_ArrayInsertFloatArray(&array2, sizeof(dataset4) / sizeof(float), dataset4);
	mt3_ArrayInsertFloatArray(&array2, sizeof(dataset5) / sizeof(float), dataset5);
	
	mt3_ArrayInsertArray(&array_arrays, array1);
	mt3_ArrayInsertArray(&array_arrays, array2);
	
	MT3_tree head = NULL;
	mt3_InsertArray(&head, "multi_array", array_arrays);
	
	SPbuffer buffer = mt3_WriteBinary(head);
	
    MT3_tree output = mt3_ReadBinary(buffer);
	
    printf("expected:\n");
    mt3_PrintTree(head);

    printf("actual:\n");
    mt3_PrintTree(output);
	
	mt3_FreeTree(&head);
	mt3_FreeArray(&array_arrays);
	mt3_FreeArray(&array1);
	mt3_FreeArray(&array2);
	mt3_FreeTree(&output);
	spBufferFree(&buffer);
}

static void test_array_of_arrays_read_write_bytes_doubles()
{
	double dataset1[] = {1.4, 2.43, 954.23, 1.32, 69.423};
	double dataset2[] = {96.8945, 965.};
	double dataset3[] = {56.35435, .5652, 182.3E+12, 45.34};
	double dataset4[] = {86.43, 1.4171, .866, .5838, 182.34};
	double dataset5[] = {6878.34, 8128, .344, 969.23, 7.423, 10.444};
	
	MT3_array array1 = NULL;
	MT3_array array2 = NULL;
	MT3_array array_arrays = NULL;
	
	mt3_ArrayInsertDoubleArray(&array1, sizeof(dataset1) / sizeof(double), dataset1);
	mt3_ArrayInsertDoubleArray(&array1, sizeof(dataset2) / sizeof(double), dataset2);
	mt3_ArrayInsertDoubleArray(&array1, sizeof(dataset3) / sizeof(double), dataset3);
	
	mt3_ArrayInsertDoubleArray(&array2, sizeof(dataset4) / sizeof(double), dataset4);
	mt3_ArrayInsertDoubleArray(&array2, sizeof(dataset5) / sizeof(double), dataset5);
	
	mt3_ArrayInsertArray(&array_arrays, array1);
	mt3_ArrayInsertArray(&array_arrays, array2);
	
	MT3_tree head = NULL;
	mt3_InsertArray(&head, "multi_array", array_arrays);
	
	SPbuffer buffer = mt3_WriteBinary(head);
	
    MT3_tree output = mt3_ReadBinary(buffer);
	
    printf("expected:\n");
    mt3_PrintTree(head);

    printf("actual:\n");
    mt3_PrintTree(output);
	
	mt3_FreeTree(&head);
	mt3_FreeArray(&array_arrays);
	mt3_FreeArray(&array1);
	mt3_FreeArray(&array2);
	mt3_FreeTree(&output);
	spBufferFree(&buffer);
}

static void test_array_of_arrays_read_write_trees()
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
	
	MT3_array list1 = NULL;
	MT3_array list2 = NULL;
	MT3_array array_arrays = NULL;
	MT3_tree head = NULL;
	
	mt3_ArrayInsertTree(&list1, tree1);
	mt3_ArrayInsertTree(&list1, tree2);
	
	mt3_ArrayInsertTree(&list2, tree3);
	mt3_ArrayInsertTree(&list2, tree4);
	mt3_ArrayInsertArray(&array_arrays, list1);
	mt3_ArrayInsertArray(&array_arrays, list2);
	/*
	mt3_InsertString(&head, "1", "hidegi");
    mt3_InsertString(&head, "2", "motex");
    mt3_InsertString(&head, "3", "betelgus");
    mt3_InsertString(&head, "4", "fjiaw");
	*/
	mt3_InsertArray(&head, "tree_list", array_arrays);
	
	
	SPbuffer buffer = mt3_WriteBinary(head);
	
    MT3_tree output = mt3_ReadBinary(buffer);
	
	
    printf("expected:\n");
    mt3_PrintTree(head);

    printf("actual:\n");
    mt3_PrintTree(output);
    printf("(%lld bytes)\n\n", buffer.length);
	
	mt3_FreeArray(&list1);
	mt3_FreeArray(&list2);
	mt3_FreeArray(&array_arrays);
	mt3_FreeTree(&head);
	mt3_FreeTree(&output);
	spBufferFree(&buffer);
}

int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
	//SP_TEST_ADD(sp_test_root_array_from_null);
	//SP_TEST_ADD(sp_test_root_array_from_alloc);
	//SP_TEST_ADD(test_byte_array_from_null);
	//SP_TEST_ADD(test_byte_array_from_alloc);
	//SP_TEST_ADD(test_array_of_arrays);
	//SP_TEST_ADD(test_byte_array_read_write);
	//SP_TEST_ADD(sp_test_root_array_from_null);
	//SP_TEST_ADD(test_array_of_arrays_read_write_bytes_ints);
	//SP_TEST_ADD(test_array_of_arrays_read_write_bytes_strings);
	//SP_TEST_ADD(test_array_of_arrays_read_write_bytes_floats);
	//SP_TEST_ADD(test_array_of_arrays_read_write_bytes_doubles);
	//SP_TEST_ADD(test_array_of_arrays_read_write_shorts);
	SP_TEST_ADD(test_array_of_arrays_read_write_trees);

	spTestRunAll();
	spTestTerminate();
	return 0;
}