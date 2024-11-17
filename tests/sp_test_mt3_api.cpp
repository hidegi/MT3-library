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
            tree = mt3_AllocObject();
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

TEST_F(MT3fixture, test_tree_deletion_medium)
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
    MT3_tree tree = NULL;
    mt3_InsertTree(&tree, "sub tree", child);

    mt3_InsertInt(&tree, "i", 1);

    mt3_InsertInt(&tree, "j", 2);

    mt3_InsertInt(&tree, "kk", 6);
    mt3_InsertInt(&tree, "ii", 6);
    mt3_InsertStringArray(&tree, "names", 4, names);
    mt3_InsertInt(&tree, "suberr", 135);
    mt3_InsertInt(&tree, "suber", 512);

	ASSERT_TRUE(mt3_Delete(&tree, "ii"));
	ASSERT_TRUE(mt3_Delete(&tree, "j"));
	ASSERT_TRUE(mt3_Delete(&tree, "names"));
	ASSERT_TRUE(mt3_Delete(&tree, "sub tree"));

    ASSERT_TRUE(tree != NULL);
	ASSERT_TRUE(mt3_VerifyRBT(tree));
    mt3_FreeTree(&child);
}


TEST_F(MT3fixture, test_search)
{
	mt3_InsertByte(&tree, "byte", 42);
    mt3_InsertShort(&tree, "short", 542);
	mt3_InsertInt(&tree, "int", 4421);
	mt3_InsertLong(&tree, "long", 9485);
	mt3_InsertFloat(&tree, "float", 1.31123);
	mt3_InsertDouble(&tree, "double", 4353.345);
	mt3_InsertString(&tree, "string", "hda");
	
	/*
	ASSERT_TRUE(mt3_Search(tree, "byte") != NULL);
	ASSERT_TRUE(mt3_Search(tree, "short") != NULL);
	ASSERT_TRUE(mt3_Search(tree, "int") != NULL);
	ASSERT_TRUE(mt3_Search(tree, "long") != NULL);
	ASSERT_TRUE(mt3_Search(tree, "float") != NULL);
	ASSERT_TRUE(mt3_Search(tree, "double") != NULL);
	ASSERT_TRUE(mt3_Search(tree, "string") != NULL);
	*/
}

TEST_F(MT3fixture, test_get_set)
{
	mt3_InsertByte(&tree, "byte", 42);
	ASSERT_TRUE(tree != NULL);
	
    mt3_InsertShort(&tree, "short", 542);
	mt3_InsertInt(&tree, "int", 4421);
	mt3_InsertLong(&tree, "long", 9485);
	mt3_InsertFloat(&tree, "float", 1.31123);
	mt3_InsertDouble(&tree, "double", 4353.345);
	mt3_InsertString(&tree, "string", "hda");
	ASSERT_TRUE(mt3_VerifyRBT(tree));
	 
	mt3_SetByte(tree, "byte", 64);
	mt3_SetShort(tree, "short", 135);
    mt3_SetInt(tree, "int", 1667);
    mt3_SetLong(tree, "long", 123456789);
	mt3_SetFloat(tree, "float", 3.141592654);
	mt3_SetDouble(tree, "double", 1.98888891);
	mt3_SetString(tree, "string", "hidegion");
	
	SPbyte b64  = mt3_GetByte(tree, "byte");
	SPshort s135 = mt3_GetShort(tree, "short");
	SPint i1667 = mt3_GetInt(tree, "int");
	SPlong l123 = mt3_GetLong(tree, "long");
	SPfloat fpi = mt3_GetFloat(tree, "float");
	SPdouble dsn = mt3_GetDouble(tree, "double");
	const SPchar* str = mt3_GetString(tree, "string");
	
	ASSERT_TRUE(64 == b64) << "byte: " + std::to_string(b64);
	ASSERT_TRUE(135 == s135) << "short: " + std::to_string(s135);
	ASSERT_TRUE(1667 == i1667) << "int: " + std::to_string(i1667);
	ASSERT_TRUE(123456789 == l123) << "long: " + std::to_string(l123);
	ASSERT_NEAR(3.14159265, fpi, 6) << "float: " + std::to_string(fpi);
	ASSERT_NEAR(1.98888891, dsn, 15) << "long: " + std::to_string(dsn);
	ASSERT_STREQ("hidegion", str) << "string: " << str;
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}