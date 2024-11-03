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
	
	motPrintTree(tree);
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
	
	motInsertLong(tree, "constant", 999);
	motInsertInt(tree, "inter", 888);
	motInsertShort(tree, "shortser", 777);
	motInsertByte(tree, "baite", 55);
	motInsertFloat(tree, "floater", 1111.45f);
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
	MOT_tree* child = motAllocTree("z");

	motInsertShort(child, "nmg0", -14543);
	motInsertByte(child, "nmg1", true);
	motInsertByte(child, "nmg2", false);
	motInsertLong(child, "nmg3", 495845);
	motInsertDouble(child, "byte_array", 1.998E+58);
	motInsertFloat(child, "byte_array1", 0.00001f);
	motInsertString(child, "name", "Hello World!!");
	motInsertTree(tree, child);
	
	motPrintTree(tree);
	motFreeTree(tree);
}

void test_tree_insert()
{
	MOT_tree* tree = motAllocTree("head");
	SP_ASSERT_NOT_NULL(tree);

    motInsertInt(tree, "x", 1);
    motInsertInt(tree, "yre", 2);
    motInsertInt(tree, "zv", 3);
	
    MOT_tree* child1 = motAllocTree("a");
    motInsertTree(tree, child1);

    MOT_tree* child2 = motAllocTree("b");
    motInsertTree(tree, child2);

    MOT_tree* child3 = motAllocTree("c");

    motPrintTree(tree);
	motFreeTree(tree);
}

void printByteArrayInBinary(const unsigned char *byteArray, size_t size) {
    for (size_t i = 0; i < size; i++) {
        unsigned char byte = byteArray[i];
        // Print the byte in binary
        for (int j = 7; j >= 0; j--) {
            // Use bitwise AND and right shift to get each bit
            printf("%d", (byte >> j) & 1);
        }
        printf("\n"); // New line after each byte
    }
}

void test_write_binary()
{
	MOT_tree* tree = motAllocTree("head");
	SP_ASSERT_NOT_NULL(tree);
	
	motInsertByte(tree, "x", 1);
	motInsertByte(tree, "y", 2);
	motInsertByte(tree, "header", 3);
	motInsertByte(tree, "headerer", 4);
	motInsertByte(tree, "fjiaw", 5);
	motInsertByte(tree, "fjiaw2", 46);
	motInsertLong(tree, "fjiaw3", 4565);
	motInsertFloat(tree, "sun", 45.34);
	motInsertString(tree, "hda", "hidegi was here..");

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

    SPbyte* bytes = (SPbyte*) motAllocChunk(sizeof(SPbyte) * 20);
    for(int i = 0; i < 20; i++)
        bytes[i] = 5 * i;
    motInsertArray(tree, "bytes", MOT_TAG_BYTE, 20, bytes);
	SP_ASSERT_NOT_NULL(motSearch(tree, "bytes"));
	
	const char* strings[] = 
	{
		"hidegi",
		"motex",
		"betelgus"
	}; //12 bytes
	motInsertArray(tree, "names", (MOT_tag) (MOT_TAG_STRING | MOT_TAG_ARRAY), 3, (const SPchar**)strings);
	
	SPbuffer buffer = motWriteBinary(tree);
	
	printf("expected:\n");
	motPrintTree(tree);
    printf("\n");
	MOT_tree* output = motReadBinary(buffer);
	printf("actual:\n");
	SP_ASSERT_NOT_NULL(output);
	motPrintTree(output);

	free(doubles);
	free(floats);
	free(shorts);
	free(bytes);
	//write data..
	SP_ASSERT_TRUE(sp::writeData("output.mot", buffer.data, buffer.length));
	
	SP_DEBUG("%lld bytes written to output.mot", buffer.length);
	spBufferFree(&buffer);
	
}

void test_string_array()
{
	MOT_tree* tree = motAllocTree("head");
	SP_ASSERT_NOT_NULL(tree);
	SPbyte* bytes = (SPbyte*) motAllocChunk(sizeof(SPbyte) * 20);
    for(int i = 0; i < 20; i++)
        bytes[i] = 5 * i;
    motInsertArray(tree, "bytes", MOT_TAG_BYTE, 20, bytes);
	SP_ASSERT_NOT_NULL(motSearch(tree, "bytes"));
	
	motInsertInt(tree, "x", 1);
	const char* strings[] = 
	{
		"hd",
		"hd"
	}; //12 bytes
	motInsertArray(tree, "names", MOT_TAG_STRING, 2, (const SPchar**)strings);
	
	SPbuffer buffer = motWriteBinary(tree);
	
	
	MOT_tree* output = motReadBinary(buffer);
	SP_ASSERT_NOT_NULL(output);
	motPrintTree(output);
	spBufferFree(&buffer);
	motFreeTree(output);
	motFreeTree(tree);
	free(bytes);
}

void test_root_insert()
{
	MOT_tree* parent = motAllocTree("parent");
	SP_ASSERT_NOT_NULL(parent);
	
	MOT_tree* child = motAllocTree("child");
	SP_ASSERT_NOT_NULL(child);
	motInsertByte(child, "byte", 2);
	motInsertTree(parent, child);
	motInsertByte(parent, "byte", 1);
	
	SPbuffer buffer = motWriteBinary(parent);
	MOT_tree* output = motReadBinary(buffer);
	SP_ASSERT_NOT_NULL(output);
	
	printf("expected:\n");
	motPrintTree(parent);
	printf("actual:\n");
	motPrintTree(output);
	motFreeTree(parent);
	motFreeTree(output);
	SP_DEBUG("%lld bytes", buffer.length);
	spBufferFree(&buffer);
}

int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
	//SP_TEST_ADD(test_serialization_trivial);
	//SP_TEST_ADD(test_if_all_available);
	//SP_TEST_ADD(test_if_all_available);
	SP_TEST_ADD(test_root_insert);

	spTestRunAll();
	spTestTerminate();
	return 0;
}