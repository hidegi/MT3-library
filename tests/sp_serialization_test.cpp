#include "SP/test/unit.h"
#include "SP/utils/io.h"
#include "SP/sparse/mt3.h"
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
	MT3_tree tree = mt3_AllocObject();
	mt3_InsertString(&tree, "x", "x");
	mt3_InsertString(&tree, "y", "y");
	mt3_InsertString(&tree, "fjiaw", "fjiaw");
	mt3_InsertString(&tree, "motex", "motex");
	mt3_InsertString(&tree, "value", "value");

	mt3_InsertString(&tree, "nmg", "nmg");
	mt3_InsertString(&tree, "nmg0", "nmg0");
	mt3_InsertString(&tree, "nmg1", "nmg1");
	mt3_InsertString(&tree, "nmg2", "nmg2");
	mt3_InsertString(&tree, "nmg3", "nmg3");
	mt3_InsertString(&tree, "byte_array", "byte_array");
	mt3_InsertString(&tree, "byte_array1", "byte_array1");

	SPbuffer buffer = mt3_WriteBinary(tree);
	MT3_tree output = mt3_ReadBinary(buffer);
	SP_ASSERT_NOT_NULL(output);


    printf("expected:\n");
    mt3_PrintTree(tree);

    printf("actual:\n");
    mt3_PrintTree(output);

    mt3_FreeTree(&tree);
    mt3_FreeTree(&output);

    SP_DEBUG("%lld bytes", buffer.length);

    spBufferFree(&buffer);
    mt3_FreeTree(&tree);
    mt3_FreeTree(&output);
}

void test_null_tree()
{
    MT3_tree tree = NULL;
    mt3_InsertString(&tree, "x", "x");
    mt3_InsertString(&tree, "y", "y");
    mt3_InsertString(&tree, "z", "z");
    mt3_InsertString(&tree, "w", "w");

    SP_ASSERT_NOT_NULL(tree);
    mt3_PrintTree(tree);
    mt3_FreeTree(&tree);
}

void test_empty_tree()
{
    MT3_tree tree = mt3_AllocObject();
    mt3_InsertString(&tree, "x", "x");
    mt3_InsertString(&tree, "y", "y");
    mt3_InsertString(&tree, "z", "z");
    mt3_InsertString(&tree, "w", "w");
    mt3_InsertInt(&tree, "hda", 34);

    SP_ASSERT_NOT_NULL(tree);
	SP_ASSERT_TRUE_WITH_ACTION(mt3_VerifyRBT(tree), mt3_FreeTree(&tree));
    mt3_PrintTree(tree);
    mt3_FreeTree(&tree);
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
    MT3_tree child = NULL;
    mt3_InsertString(&child, "x", "sub_x");
    mt3_InsertString(&child, "y", "sub_y");
    mt3_InsertString(&child, "z", "sub_z");
    mt3_InsertString(&child, "w", "sub_w");

    int array[] = {1, 3, 2, 1, 5, 2};
    mt3_InsertIntArray(&child, "names", 6, array);
    mt3_InsertStringArray(&child, "names", 4, names);
    MT3_tree parent = NULL;
    mt3_InsertTree(&parent, "sub tree", child);

    mt3_InsertInt(&parent, "i", 1);
    mt3_InsertInt(&parent, "j", 2);

    mt3_InsertInt(&parent, "kk", 6);
    mt3_InsertInt(&parent, "ii", 6);
    mt3_InsertStringArray(&parent, "names", 4, names);
    mt3_InsertInt(&parent, "suberr", 135);

    SPbuffer buffer = mt3_WriteBinary(parent);
    MT3_tree output = mt3_ReadBinary(buffer);

    printf("expected:\n");
    mt3_PrintTree(parent);

    printf("actual:\n");
    mt3_PrintTree(output);

    printf("(%lld bytes)\n\n", buffer.length);
    mt3_FreeTree(&parent);
    mt3_FreeTree(&child);
    mt3_FreeTree(&output);
    spBufferFree(&buffer);
}


void test_tree_deletion()
{
    MT3_tree tree = mt3_AllocObject();
    mt3_InsertString(&tree, "x", "x");
    mt3_InsertString(&tree, "y", "y");
    mt3_InsertString(&tree, "z", "z");
    mt3_InsertString(&tree, "w", "w");
    mt3_InsertInt(&tree, "hda", 34);
	SP_ASSERT_TRUE_WITH_ACTION(mt3_Delete(&tree, "w"), mt3_FreeTree(&tree));
	SP_ASSERT_TRUE_WITH_ACTION(mt3_Delete(&tree, "y"), mt3_FreeTree(&tree));

    SP_ASSERT_NOT_NULL_WITH_ACTION(tree, {mt3_PrintTree(tree); mt3_FreeTree(&tree);});
	SP_ASSERT_TRUE_WITH_ACTION(mt3_VerifyRBT(tree), mt3_FreeTree(&tree));
    mt3_PrintTree(tree);
    mt3_FreeTree(&tree);
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
    MT3_tree child = NULL;
    mt3_InsertString(&child, "x", "sub_x");
    mt3_InsertString(&child, "y", "sub_y");
    mt3_InsertString(&child, "z", "sub_z");
    mt3_InsertString(&child, "w", "sub_w");


    int array[] = {1, 3, 2, 1, 5, 2};

    mt3_InsertIntArray(&child, "numbers", 6, array);

    mt3_InsertStringArray(&child, "names", 4, names);
    MT3_tree parent = NULL;
    mt3_InsertTree(&parent, "sub tree", child);

    mt3_InsertInt(&parent, "i", 1);

    mt3_InsertInt(&parent, "j", 2);

    mt3_InsertInt(&parent, "kk", 6);
    mt3_InsertInt(&parent, "ii", 6);
    mt3_InsertStringArray(&parent, "names", 4, names);
    mt3_InsertInt(&parent, "suberr", 135);
    mt3_InsertInt(&parent, "suber", 512);

	SP_ASSERT_TRUE_WITH_ACTION(mt3_Delete(&parent, "ii"), mt3_FreeTree(&parent));
	SP_ASSERT_TRUE_WITH_ACTION(mt3_Delete(&parent, "j"), mt3_FreeTree(&parent));
	SP_ASSERT_TRUE_WITH_ACTION(mt3_Delete(&parent, "names"), mt3_FreeTree(&parent));
	SP_ASSERT_TRUE_WITH_ACTION(mt3_Delete(&parent, "sub tree"), mt3_FreeTree(&parent));

    SP_ASSERT_NOT_NULL_WITH_ACTION(parent, {mt3_PrintTree(parent); mt3_FreeTree(&parent);});
	SP_ASSERT_TRUE_WITH_ACTION(mt3_VerifyRBT(parent), mt3_FreeTree(&parent));

    mt3_PrintTree(parent);
    mt3_FreeTree(&parent);
    mt3_FreeTree(&child);
}

#define ITERATIONS 100
#define LENGTH 200

void test_tree_search()
{
	std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dst(1,1000);
	MT3_tree tree = NULL;

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
		mt3_InsertInt(&tree, std::to_string(array[i]).c_str(), array[i]);
	}

	for(int i = 0; i < LENGTH; i++)
    {
		SP_ASSERT_NOT_NULL_WITH_ACTION(mt3_Search(tree, std::to_string(array[i]).c_str()), mt3_FreeTree(&tree));
	}

	mt3_FreeTree(&tree);
	delete[] array;
}

SPindex getRandomIndex(bool array[], SPsize length) {
	SPindex index = -1;
	SPindex* used = new SPindex[length];
	SPsize indexCount = 0;

	for(SPsize i = 0; i < length; i++)
	{
		if(!array[i])
			used[indexCount++] = i;
	}

	if(indexCount == 0)
	{
	    delete [] used;
		return -1;
    }

	index = used[std::rand() % (indexCount)];
	array[index] = true;
	delete [] used;

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
        MT3_tree tree = NULL;
		
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
            mt3_InsertInt(&tree, std::to_string(array[i]).c_str(), array[i]);
        }
		
		
        SP_ASSERT_TRUE_WITH_ACTION(mt3_VerifyRBT(tree), {mt3_FreeTree(&tree);});
		
		SPsize index = -1; 
		do
		{
			index = getRandomIndex(cache, LENGTH);
			
			if(index != -1)
			{
				bool status = mt3_Delete(&tree, std::to_string(array[index]).c_str());
				SP_ASSERT_TRUE_WITH_ACTION(status,
				{
					SP_DEBUG("failed to delete %d", array[index]);
					mt3_PrintTree(tree);
					delete [] array;
					mt3_FreeTree(&tree);
				});
				
				SP_ASSERT_TRUE_WITH_ACTION(mt3_VerifyRBT(tree),
				{
					SP_DEBUG("imbalanced tree for %d", array[index]);
					delete [] array;
					mt3_FreeTree(&tree);
				});
			}
			
		} while(index != -1);
		
        SP_ASSERT_TRUE_WITH_ACTION(mt3_VerifyRBT(tree), mt3_FreeTree(&tree));
        mt3_FreeTree(&tree);
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
	SP_TEST_ADD(test_sub_tree);
	//SP_TEST_ADD(test_tree_random_integers);
	//SP_TEST_ADD(test_random_index);

	spTestRunAll();
	spTestTerminate();
	return 0;
}