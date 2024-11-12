#include "SP/test/unit.h"
#include "SP/utils/io.h"
#include "SP/sparse/mot.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

bool containsNumber(int n, const int* array, int length)
{
    for(int i = 0; i < length; i++)
        if(array[i] == n)
            return true;
    return false;
}

void test_insertion()
{
	MOT_tree tree = motAllocTree();
	motInsertString(&tree, "x", "x");
	motInsertString(&tree, "y", "y");
	motInsertString(&tree, "fjiaw", "fjiaw");
	motInsertString(&tree, "motex", "motex");
	motInsertString(&tree, "value", "value");

	motInsertString(&tree, "nmg", "nmg");
	motInsertString(&tree, "nmg0", "nmg0");
	motInsertString(&tree, "nmg1", "nmg1");
	motInsertString(&tree, "nmg2", "nmg2");
	motInsertString(&tree, "nmg3", "nmg3");
	motInsertString(&tree, "byte_array", "byte_array");
	motInsertString(&tree, "byte_array1", "byte_array1");

	SPbuffer buffer = motWriteBinary(tree);
	MOT_tree output = motReadBinary(buffer);
	SP_ASSERT_NOT_NULL(output);


    printf("expected:\n");
    motPrintTree(tree);

    printf("actual:\n");
    motPrintTree(output);

    motFreeTree(tree);
    motFreeTree(output);

    SP_DEBUG("%lld bytes", buffer.length);

    spBufferFree(&buffer);
    motFreeTree(tree);
    motFreeTree(output);
}

void test_null_tree()
{
    MOT_tree tree = NULL;
    motInsertString(&tree, "x", "x");
    motInsertString(&tree, "y", "y");
    motInsertString(&tree, "z", "z");
    motInsertString(&tree, "w", "w");

    SP_ASSERT_NOT_NULL(tree);
    motPrintTree(tree);
    motFreeTree(tree);
}

void test_empty_tree()
{
    MOT_tree tree = motAllocTree();
    motInsertString(&tree, "x", "x");
    motInsertString(&tree, "y", "y");
    motInsertString(&tree, "z", "z");
    motInsertString(&tree, "w", "w");
    motInsertInt(&tree, "hda", 34);
	
    SP_ASSERT_NOT_NULL(tree);
	SP_ASSERT_TRUE_WITH_ACTION(motVerifyRBT(tree), motFreeTree(tree));
    motPrintTree(tree);
    motFreeTree(tree);
}

void test_sub_tree()
{
    const char* names[] =
    {
        "hidegi",
        "motex",
        "betelgus",
        "fjiaw"
    };
    MOT_tree child = NULL;
    motInsertString(&child, "x", "sub_x");
    motInsertString(&child, "y", "sub_y");
    motInsertString(&child, "z", "sub_z");
    motInsertString(&child, "w", "sub_w");

    int array[] = {1, 3, 2, 1, 5, 2};
    //motInsertArray(&child, "names", MOT_TAG_INT, 6, array);
    motInsertArray(&child, "names", MOT_TAG_STRING, 4, names);
    MOT_tree parent = NULL;
    motInsertTree(&parent, "sub tree", child);

    motInsertInt(&parent, "i", 1);
    motInsertInt(&parent, "j", 2);

    motInsertInt(&parent, "kk", 6);
    motInsertInt(&parent, "ii", 6);
    motInsertArray(&parent, "names", MOT_TAG_STRING, 4, names);
    motInsertInt(&parent, "suberr", 135);

    SPbuffer buffer = motWriteBinary(parent);
    MOT_tree output = motReadBinary(buffer);

    printf("expected:\n");
    motPrintTree(parent);

    printf("actual:\n");
    motPrintTree(output);

    printf("(%lld bytes)\n\n", buffer.length);
    motFreeTree(parent);
    motFreeTree(output);
    spBufferFree(&buffer);
}


void test_tree_deletion()
{
    MOT_tree tree = motAllocTree();
    motInsertString(&tree, "x", "x");
    motInsertString(&tree, "y", "y");
    motInsertString(&tree, "z", "z");
    motInsertString(&tree, "w", "w");
    motInsertInt(&tree, "hda", 34);
	SP_ASSERT_TRUE_WITH_ACTION(motDelete(&tree, "w"), motFreeTree(tree));
	SP_ASSERT_TRUE_WITH_ACTION(motDelete(&tree, "y"), motFreeTree(tree));
	
    SP_ASSERT_NOT_NULL_WITH_ACTION(tree, {motPrintTree(tree); motFreeTree(tree);});
	SP_ASSERT_TRUE_WITH_ACTION(motVerifyRBT(tree), motFreeTree(tree));
    motPrintTree(tree);
    motFreeTree(tree);
}

void test_tree_deletion_medium()
{
    const char* names[] =
    {
        "hidegi",
        "motex",
        "betelgus",
        "fjiaw"
    };
    MOT_tree child = NULL;
    motInsertString(&child, "x", "sub_x");
    motInsertString(&child, "y", "sub_y");
    motInsertString(&child, "z", "sub_z");
    motInsertString(&child, "w", "sub_w");

    int array[] = {1, 3, 2, 1, 5, 2};
    //motInsertArray(&child, "names", MOT_TAG_INT, 6, array);
    motInsertArray(&child, "names", MOT_TAG_STRING, 4, names);
    MOT_tree parent = NULL;
    motInsertTree(&parent, "sub tree", child);

    motInsertInt(&parent, "i", 1);
    motInsertInt(&parent, "j", 2);

    motInsertInt(&parent, "kk", 6);
    motInsertInt(&parent, "ii", 6);
    motInsertArray(&parent, "names", MOT_TAG_STRING, 4, names);
    motInsertInt(&parent, "suberr", 135);
	SP_ASSERT_TRUE_WITH_ACTION(motDelete(&parent, "ii"), motFreeTree(parent));
	SP_ASSERT_TRUE_WITH_ACTION(motDelete(&parent, "j"), motFreeTree(parent));
	SP_ASSERT_TRUE_WITH_ACTION(motDelete(&parent, "names"), motFreeTree(parent));
	SP_ASSERT_TRUE_WITH_ACTION(motDelete(&parent, "sub tree"), motFreeTree(parent));
	
    SP_ASSERT_NOT_NULL_WITH_ACTION(parent, {motPrintTree(parent); motFreeTree(parent);});
	SP_ASSERT_TRUE_WITH_ACTION(motVerifyRBT(parent), motFreeTree(parent));
    motPrintTree(parent);
    motFreeTree(parent);
}

#define ITERATIONS 100
#define LENGTH 100

void test_tree_search()
{
	std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dst(1,1000);
	MOT_tree tree = NULL;
	
	int* array = new int[LENGTH];
	for(int i = 0; i < LENGTH; i++)
    {
		int num; 
		do
		{
			num = dst(rng);
		}
		while(containsNumber(num, array, LENGTH));
		array[i] = num;
		motInsertInt(&tree, std::to_string(array[i]).c_str(), array[i]);
	}
	
	for(int i = 0; i < LENGTH; i++)
    {
		SP_ASSERT_NOT_NULL_WITH_ACTION(motSearch(tree, std::to_string(array[i]).c_str()), motFreeTree(tree));
	}
	
	motFreeTree(tree);
	delete[] array;
}

SPindex getRandomIndex(bool array[], SPsize length) {
	SPindex index = -1;
	SPindex used[length] = {};
	SPsize indexCount = 0;
	
	for(SPsize i = 0; i < length; i++)
	{
		if(!array[i])
			used[indexCount++] = i;
	}
	
	if(indexCount == 0)
		return -1;
	index = used[std::rand() % (indexCount)];
	array[index] = true;
    return index;
}

void test_random_index()
{
	constexpr int length = 100;
	bool array[length] = {};
	memset(array, false, sizeof(bool) * length);
	
	SPindex index = -1;
	SPsize checksum = static_cast<SPsize>((length) * (length - 1) / 2.0);
	SPsize s = 0;
	do
	{
		index = getRandomIndex(array, length);
		if(index != -1)
		{
			s += index;
		}
	}
	while(index != -1);
	
	SP_ASSERT_INTEGER_EQUAL(checksum, s);
}

void test_tree_random_integers()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dst(1,1000);
	
    for(int i = 0; i < ITERATIONS; i++)
    {
        MOT_tree tree = NULL;
		
		bool cache[LENGTH] = {false};
		memset(cache, false, sizeof(bool) * LENGTH);
        int* array = new int[LENGTH];
        for(int i = 0; i < LENGTH; i++)
        {
            int num; 
			do
			{
				num = dst(rng);
			}
            while(containsNumber(num, array, LENGTH));
                
            array[i] = num;
            motInsertInt(&tree, std::to_string(array[i]).c_str(), array[i]);
        }
		
		
        SP_ASSERT_TRUE_WITH_ACTION(motVerifyRBT(tree), {motFreeTree(tree);});
		
		SPsize index = -1; 
		do
		{
			index = getRandomIndex(cache, LENGTH);
			
			if(index != -1)
			{
				bool status = motDelete(&tree, std::to_string(array[index]).c_str());
				SP_ASSERT_TRUE_WITH_ACTION(status,
				{
					SP_DEBUG("failed to delete %d", array[index]);
					motPrintTree(tree);
					delete [] array;
					motFreeTree(tree);
				});
				
				SP_ASSERT_TRUE_WITH_ACTION(motVerifyRBT(tree),
				{
					SP_DEBUG("imbalanced tree for %d", array[index]);
					delete [] array;
					motFreeTree(tree);
				});
			}
			
		} while(index != -1);
		
        SP_ASSERT_TRUE_WITH_ACTION(motVerifyRBT(tree), motFreeTree(tree));
        motFreeTree(tree);
        delete[] array;
	}
	SP_DEBUG("DONE: %d iteration(s) with %d insertion(s)/deletion(s)", ITERATIONS, LENGTH);
}
int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
	//SP_TEST_ADD(test_serialization_trivial);
	//SP_TEST_ADD(test_if_all_available);
	//SP_TEST_ADD(test_if_all_available);
	//SP_TEST_ADD(test_insertion);
	//SP_TEST_ADD(test_null_tree);
	SP_TEST_ADD(test_tree_random_integers);
	//SP_TEST_ADD(test_random_index);

	spTestRunAll();
	spTestTerminate();
	return 0;
}