#include "SP/sparse/mot.h"
#include <gtest/gtest.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

class MotFixture : public testing::Test
{
    public:
        MotFixture()
        {
        }

       ~MotFixture()
        {
        }
    protected:
        void SetUp() override
        {
            tree = motAllocTree();
            std::cout << ">>>" << __PRETTY_FUNCTION__ << " has been run" << std::endl;
        }

        void TearDown() override
        {
            motFreeTree(&tree);
            std::cout << ">>>" << __PRETTY_FUNCTION__ << " has been run" << std::endl;
        }

    public:
        MOT_tree tree;
};

static bool containsNumber(int n, const int* array, int length)
{
    for(int i = 0; i < length; i++)
        if(array[i] == n)
            return true;
    return false;
}

static SPindex getRandomIndex(bool array[], SPsize length) {
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

TEST_F(MotFixture, test_insertion)
{
    //act..
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

    //assert..
    ASSERT_TRUE(motVerifyRBT(tree));
    ASSERT_EQ(MOT_STATUS_OK, motGetLastError());
}

#define ITERATIONS 100
#define LENGTH 200
TEST_F(MotFixture, test_tree_random_integers)
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

        ASSERT_TRUE(motVerifyRBT(tree)) << "Imbalanced tree";

		SPsize index = -1;
		do
		{
			index = getRandomIndex(cache, LENGTH);
			if(index != -1)
			{
				ASSERT_TRUE(motDelete(&tree, std::to_string(array[index]).c_str())) << "Deletion failed";
				ASSERT_TRUE(motVerifyRBT(tree)) << "Imbalanced tree";
			}

		} while(index != -1);

        ASSERT_TRUE(motVerifyRBT(tree)) << "Imbalanced tree";
        motFreeTree(&tree);
        delete[] array;
	}
	ASSERT_EQ(MOT_STATUS_OK, motGetLastError());
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}