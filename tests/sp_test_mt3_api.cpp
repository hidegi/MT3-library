#include "SP/sparse/mt3.h"
#include <gtest/gtest.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

class MT3fixture : public testing::Test
{
    public:
        MT3fixture()
        {
        }

       ~MT3fixture()
        {
        }
    protected:
        void SetUp() override
        {
            tree = mt3_AllocTree();
            std::cout << ">>>" << __PRETTY_FUNCTION__ << " has been run" << std::endl;
        }

        void TearDown() override
        {
            mt3_FreeTree(&tree);
            std::cout << ">>>" << __PRETTY_FUNCTION__ << " has been run" << std::endl;
        }

    public:
        MT3_tree tree;
};

static bool containsNumber(int n, const int* array, int length)
{
    for(int i = 0; i < length; i++)
        if(array[i] == n)
            return true;
    return false;
}

static SPindex getRandomIndex(bool array[], SPsize length)
{
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

//arrange
TEST_F(MT3fixture, test_insertion)
{
    //act..
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

    //assert..
    ASSERT_TRUE(mt3_VerifyRBT(tree));
    ASSERT_EQ(MT3_STATUS_OK, mt3_GetLastError());
}

#define ITERATIONS 100
#define LENGTH 200
TEST_F(MT3fixture, test_tree_random_integers)
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

        ASSERT_TRUE(mt3_VerifyRBT(tree)) << "Imbalanced tree";

		SPsize index = -1;
		do
		{
			index = getRandomIndex(cache, LENGTH);
			if(index != -1)
			{
				ASSERT_TRUE(mt3_Delete(&tree, std::to_string(array[index]).c_str())) << "Deletion failed";
				ASSERT_TRUE(mt3_VerifyRBT(tree)) << "Imbalanced tree";
			}

		} while(index != -1);

        ASSERT_TRUE(mt3_VerifyRBT(tree)) << "Imbalanced tree";
        mt3_FreeTree(&tree);
        delete[] array;
	}
	ASSERT_EQ(MT3_STATUS_OK, mt3_GetLastError());
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}