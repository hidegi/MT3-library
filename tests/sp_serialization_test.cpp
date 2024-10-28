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
	motInsertDouble(tree, "byte_array", 1.998E+58);
	motInsertFloat(tree, "byte_array1", 0.00001f);
	//motInsertString(tree, "hidegion", "hidegion");
	
	/*
	motInsertString(tree, "byte_array", "motex");
	motInsertString(tree, "byte_array1", "hidegi");
	motInsertString(tree, "byte_array2", "motex");
	motInsertString(tree, "byte_array_2", "hidegi");
	*/
	
	MOT_byte_array ba;
	ba.length = 10;
	ba.data = (SPbyte*) malloc(ba.length);
	for(int i = 0; i < ba.length; i++)
		ba.data[i] = i;
	motInsertByteArray(tree, "byte_array", ba);
	/*
	ba.length = 10;
	ba.data = (SPbyte*) malloc(ba.length);
	
	for(int i = 0; i < ba.length; i++)
		ba.data[i] = i * 2;
	motInsertByteArray(tree, "byte_array1", ba);
	motInsertByteArray(tree, "byte_array2", ba);
	motInsertByteArray(tree, "byte_array_2", ba);
	*/
	
	MOT_int_array ia = motAllocIntArray(10);
	for(long i = 1; i <= ia.length; i++)
		ia.data[i] = i;
	motInsertIntArray(tree, "int_array", ia);
	
	MOT_tree* node = motSearch(tree, "int_array");
	SP_ASSERT_NOT_NULL(node);
	
	MOT_tag tag = node->tag;
	SP_ASSERT_EQUAL(MOT_TAG_INT, tag & 0x1FF);
	SP_ASSERT_NOT_EQUAL(0, tag & MOT_TAG_ARRAY);
	
	MOT_long_array la = motAllocLongArray(10);
	la.data[0] = 1;
	for(long i = 0; i < la.length; i++)
		la.data[i] = i * 2;
	
	SP_DEBUG("long array ok");
	motInsertLongArray(tree, "factorial", la);
	
	SP_ASSERT_NOT_NULL(motSearch(tree, "byte_array"));
	motInsertFloat(tree, "floatex", 3.141592654f);
	motPrintTree(tree);
	free(ba.data);
	free(ia.data);
	free(la.data);
	motFreeTree(tree);
}

void test_if_all_available()
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
	motInsertDouble(tree, "byte_array", 1.998E+58);
	motInsertFloat(tree, "byte_array1", 0.00001f);
	
	
	SP_ASSERT_NOT_NULL(motSearch(tree, "x"));
	SP_ASSERT_NULL(motSearch(tree, "hda1"));
	SP_ASSERT_NOT_NULL(motSearch(tree, "byte_array"));
	SP_ASSERT_NOT_NULL(motSearch(tree, "head"));
	
	motFreeTree(tree);
}
int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
	SP_TEST_ADD(test_serialization_trivial);
	//SP_TEST_ADD(test_if_all_available);

	spTestRunAll();
	spTestTerminate();
	return 0;
}