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
	SPshort* shorts = (SPshort*) motAllocChunk(sizeof(SPshort) * 10);
	for(int i = 0; i < 10; i++)
		shorts[i] = 1666 + i;
	
	motInsertArray(tree, "shorts", MOT_TAG_SHORT, 10, shorts);
	SP_ASSERT_NOT_NULL(motSearch(tree, "shorts"));
	
	motPrintTree(tree);
	free(ba.data);
	free(ia.data);
	free(la.data);
	free(shorts);
	motFreeTree(tree);
}

void test_insert_generic_array()
{
	MOT_tree* tree = motAllocTree("head");
	SP_ASSERT_NOT_NULL(tree);
	
	
	SPshort* shorts = (SPshort*) motAllocChunk(sizeof(SPshort) * 10);
	for(int i = 0; i < 10; i++)
		shorts[i] = 1666 + i;
	motInsertArray(tree, "shorts", MOT_TAG_SHORT, 10, shorts);
	SP_ASSERT_NOT_NULL(motSearch(tree, "shorts"));
	
	SPfloat* floats = (SPfloat*) motAllocChunk(sizeof(SPfloat) * 10);
	for(int i = 0; i < 10; i++)
		floats[i]= 3.45 * 10.f * i;
	motInsertArray(tree, "floats", MOT_TAG_FLOAT, 10, floats);
	SP_ASSERT_NOT_NULL(motSearch(tree, "floats"));
	
	SPdouble* doubles= (SPdouble*) motAllocChunk(sizeof(SPdouble) * 10);
	for(int i = 0; i < 10; i++)
		doubles[i]= 0.3f * i * i + 1.4 * i + 5.2;
	
	motInsertArray(tree, "doubles", MOT_TAG_DOUBLE, 10, doubles);
	SP_ASSERT_NOT_NULL(motSearch(tree, "doubles"));
	
	motInsertLong(tree, "constant", 10);
	motInsertInt(tree, "inter", 10);
	motInsertShort(tree, "shortser", 10);
	motInsertByte(tree, "baite", 10);
	motInsertFloat(tree, "floater", 10.45f);
	motInsertDouble(tree, "doubleman", 435.344);
	motInsertString(tree, "hda", "hidegious hidegi was here..");
	printf("\n");
	motPrintTree(tree);
	motFreeTree(tree);
	free(shorts);
	free(floats);
	free(doubles);
	
	
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
	//SP_TEST_ADD(test_serialization_trivial);
	//SP_TEST_ADD(test_if_all_available);
	SP_TEST_ADD(test_insert_generic_array);

	spTestRunAll();
	spTestTerminate();
	return 0;
}