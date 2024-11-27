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
	MT3_tree tree = mt3_AllocTree();
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
    MT3_tree tree = mt3_AllocTree();
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
	mt3_InsertString(&parent, "hda", "hda");
	mt3_SetInt(parent, "kk", 137);
    mt3_InsertStringArray(&parent, "names", 4, names);
    mt3_InsertInt(&parent, "suberr", 135);
	
	mt3_SetString(parent, "hda", "hidegion");
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
    MT3_tree tree = mt3_AllocTree();
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
	
	/*
	for(int i = 0; i < LENGTH; i++)
    {
		SP_ASSERT_NOT_NULL_WITH_ACTION(mt3_Search(tree, std::to_string(array[i]).c_str()), mt3_FreeTree(&tree));
	}
	*/
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

void test_get_set()
{
	MT3_tree tree = NULL;
	mt3_InsertByte(&tree, "byte", 42);
	SP_ASSERT_NOT_NULL(tree);
	
    mt3_InsertShort(&tree, "short", 542);
	mt3_InsertInt(&tree, "int", 4421);
	mt3_InsertLong(&tree, "long", 9485);
	mt3_InsertFloat(&tree, "float", 1.31123);
	mt3_InsertDouble(&tree, "double", 4353.345);
	mt3_InsertString(&tree, "string", "hda");
	SP_ASSERT_TRUE_WITH_ACTION(mt3_VerifyRBT(tree), mt3_FreeTree(&tree));
	
	mt3_SetByte(tree, "byte", 64);
	mt3_SetShort(tree, "short", 135);
    mt3_SetInt(tree, "int", 1667);
    mt3_SetLong(tree, "long", 123456789);
	mt3_SetFloat(tree, "float", 3.141592654);
	mt3_SetDouble(tree, "double", 1.988E+30);
	mt3_SetString(tree, "string", "hidegion");
	
	SPbyte b64  = mt3_GetByte(tree, "byte");
	SPshort s135 = mt3_GetShort(tree, "short");
	SPint i1667 = mt3_GetInt(tree, "int");
	SPlong l123 = mt3_GetLong(tree, "long");
	SPfloat fpi = mt3_GetFloat(tree, "float");
	SPdouble dsn = mt3_GetDouble(tree, "double");
	const SPchar* str = mt3_GetString(tree, "string");
	
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(64, b64, mt3_FreeTree(&tree));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(135, s135, mt3_FreeTree(&tree));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(1667, i1667, mt3_FreeTree(&tree));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(123456789, l123, mt3_FreeTree(&tree));
	SP_ASSERT_DECIMAL_ALMOST_EQUAL_WITH_ACTION(3.14159265, fpi, 6, mt3_FreeTree(&tree));
	SP_ASSERT_DECIMAL_ALMOST_EQUAL_WITH_ACTION(1.988E+30, dsn, 6, mt3_FreeTree(&tree));
	SP_ASSERT_STRING_EQUAL_WITH_ACTION("hidegion", str, mt3_FreeTree(&tree));
	printf("byte: %d\nshort: %d\nint: %d\nlong: %lld\nfloat: %f\ndouble: %f\nstring: %s\n\n",
		b64, s135, i1667, l123, fpi, dsn, str);
	mt3_PrintTree(tree);
	mt3_FreeTree(&tree);
}

// assuming that access-string has form of "a.b.c.etc.."
/*
 *	if empty string, return head, otherwise,
 *	1. extract first token
 *	2. search for a child with such token..
 *		if no such child exists, return head..
 *	
 *	3. deepSearch further with remaining tokens..
 */
MT3_tree* deepSearch(const MT3_tree* head, const std::string& token)
{
	// if it is empty, then return head..
	if(!head)
		return NULL;
	
	if(token.empty())
	{
		return const_cast<MT3_tree*>(head);
	}
	
	auto pos = token.find_first_of('.');
	if(pos == std::string::npos)
	{
		return mt3_GetTree(*head, token.c_str());
	}
	
	std::string branch = token.substr(0, pos);
	MT3_tree* child = mt3_GetTree(*head, branch.c_str());
	if(!child)
	{
		return NULL;
	}
	std::string children = token.substr(pos + 1, token.size() - pos);
	return deepSearch(child, children);
}

void test_deep_search()
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
	
	MT3_tree child2 = NULL;
	mt3_InsertString(&child2, "a", "sub_a");
    mt3_InsertString(&child2, "b", "sub_b");
    mt3_InsertString(&child2, "c", "sub_c");
    mt3_InsertString(&child2, "d", "sub_d");
	mt3_InsertTree(&child, "child123", child2);

    int array[] = {1, 3, 2, 1, 5, 2};
    mt3_InsertIntArray(&child, "numbers", 6, array);
    mt3_InsertStringArray(&child, "names", 4, names);
    MT3_tree parent = NULL;
    mt3_InsertTree(&parent, "child", child);

    mt3_InsertInt(&parent, "i", 1);
    mt3_InsertInt(&parent, "j", 2);

    mt3_InsertInt(&parent, "kk", 6);
    mt3_InsertInt(&parent, "ii", 6);
	mt3_InsertString(&parent, "hda", "hda");
	mt3_SetInt(parent, "kk", 137);
    mt3_InsertStringArray(&parent, "names", 4, names);
    mt3_InsertInt(&parent, "suberr", 135);
	
	MT3_tree* branch = deepSearch(&parent, "child");
	SP_ASSERT_TRUE_WITH_ACTION(branch && *branch, 
		mt3_FreeTree(&parent);
		mt3_FreeTree(&child);
		mt3_FreeTree(&child2);
	);
	
	mt3_InsertString(branch, "motex", "motex gaming 1667");
	branch = deepSearch(&parent, "child.child123");
	SP_ASSERT_TRUE_WITH_ACTION(branch && *branch, 
		mt3_FreeTree(&parent);
		mt3_FreeTree(&child);
		mt3_FreeTree(&child2);
	);
	mt3_SetString(*branch, "a", "hidegion was here");
	mt3_PrintTree(parent);
	mt3_FreeTree(&parent);
	mt3_FreeTree(&child);
	mt3_FreeTree(&child2);
}

std::string extractMember(const std::string& str)
{
	auto dot = str.find_last_of(".");
	if(dot == std::string::npos)
		return str;
	
	return str.substr(dot + 1, str.size());
}

std::string extractDomain(const std::string& str)
{
	auto dot = str.find_last_of(".");
	if(dot == std::string::npos)
		return str;
	
	return str.substr(0, dot);
}

void test_name_extraction()
{
	std::string selection = "head.child.childXYZ.shark.x";
	std::string member = extractMember(selection);
	SP_ASSERT_TRUE_WITH_ACTION(member == "x", SP_DEBUG("member: %s", member.c_str()););
	
	selection = "x";
	member = extractMember(selection);
	SP_ASSERT_TRUE_WITH_ACTION(member == "x", SP_DEBUG("member: %s", member.c_str()););
	
	selection = "head.child.childXYZ.shark.nmg9.betelgus.hda1666.fjiaw";
	member = extractMember(selection);
	SP_ASSERT_TRUE_WITH_ACTION(member == "fjiaw", SP_DEBUG("member: %s", member.c_str()););
	
	std::string domain = extractDomain(selection);
	SP_ASSERT_TRUE_WITH_ACTION(domain == "head.child.childXYZ.shark.nmg9.betelgus.hda1666" , SP_DEBUG("domain: %s", domain.c_str()););
	
	selection = "head";
	domain = extractDomain(selection);
	member = extractMember(selection);
	SP_ASSERT_TRUE(member == "head" && domain == "head");
}

void test_insert_empty_tree()
{
	const char* names[] =
    {
        "hidegi",
        "motex",
        "betelgus",
        "fjiaw"
    };
	MT3_tree parent = NULL;
    mt3_InsertTree(&parent, "child", NULL);
	SP_ASSERT_NOT_NULL(parent);
	
	MT3_tree* child = mt3_GetTree(parent, "child");
	SP_ASSERT_NOT_NULL_WITH_ACTION(child, mt3_FreeTree(&parent));
	
	mt3_InsertInt(child, "xda", 1);
	mt3_InsertTree(child, "child123", NULL);
	mt3_InsertString(child, "x", "sub_x");
    mt3_InsertString(child, "y", "sub_y");
    mt3_InsertString(child, "z", "sub_z");
    mt3_InsertString(child, "w", "sub_w");
	
	MT3_tree* child123 = mt3_GetTree(*child, "child123");
	mt3_InsertString(child123, "a", "sub_a");
    mt3_InsertString(child123, "b", "sub_b");
    mt3_InsertString(child123, "c", "sub_c");
    mt3_InsertString(child123, "d", "sub_d");
	
	int array[] = {1, 3, 2, 1, 5, 2};
    mt3_InsertIntArray(child, "numbers", 6, array);
    mt3_InsertStringArray(child, "names", 4, names);
	
	mt3_SetString(*child123, "a", "hidegion was here");
	const char* str = mt3_GetString(*child123, "a");
	
	SP_ASSERT_STRING_EQUAL_WITH_ACTION("hidegion was here", str, mt3_FreeTree(&parent));
	mt3_SetString(*child123, "a", "hidegion was here");
	
	mt3_PrintTree(parent);
	mt3_FreeTree(&parent);
}

static void big_test()
{
	SPbyte byte_dataset1[] = {-76, -17, 33, 109, -3, 42, -42, 62, 116, 34};
	SPbyte byte_dataset2[] = {-39, 47, -59, 4, 97, 59, 8, 11, -90, -10};
	SPbyte byte_dataset3[] = {-41, -79, -52, 61, 98, -66, 81, 75, -100, 113};

	SPbyte byte_dataset4[] = {-115, -76, 113, 57, 120, 59, 52, -89, 51, -118};
	SPbyte byte_dataset5[] = {69, 18, -94, -50, -43, -28, -108, -7, -106, 40};
	SPbyte byte_dataset6[] = {11, -43, -92, -52, -65, -27, 109, 99, 6, 91};

	SPbyte byte_dataset7[] = {52, -65, -90, 80, 54, -22, 117, 90, -8, -92};
	SPbyte byte_dataset8[] = {-96, -11, -103, 42, -78, -71, -59, -90, 21, -47};
	SPbyte byte_dataset9[] = {119, -31, -73, 90, 31, -80, 86, -100, 106, 13};

    SPshort short_dataset1[] = {-76, -17, 33, 109, -3, 42, -42, 62, 116, 34};
	SPshort short_dataset2[] = {-39, 47, -59, 4, 97, 59, 8, 11, -90, -10};
	SPshort short_dataset3[] = {-41, -79, -52, 61, 98, -66, 81, 75, -100, 113};

	SPshort short_dataset4[] = {-115, -76, 113, 57, 120, 59, 52, -89, 51, -118};
	SPshort short_dataset5[] = {69, 18, -94, -50, -43, -28, -108, -7, -106, 40};
	SPshort short_dataset6[] = {11, -43, -92, -52, -65, -27, 109, 99, 6, 91};

	SPshort short_dataset7[] = {52, -65, -90, 80, 54, -22, 117, 90, -8, -92};
	SPshort short_dataset8[] = {-96, -11, -103, 42, -78, -71, -59, -90, 21, -47};
	SPshort short_dataset9[] = {119, -31, -73, 90, 31, -80, 86, -100, 106, 13};


	SPint int_dataset1[] = {-76, -17, 33, 109, -3, 42, -42, 62, 116, 34};
	SPint int_dataset2[] = {-39, 47, -59, 4, 97, 59, 8, 11, -90, -10};
	SPint int_dataset3[] = {-41, -79, -52, 61, 98, -66, 81, 75, -100, 113};

	SPint int_dataset4[] = {-115, -76, 113, 57, 120, 59, 52, -89, 51, -118};
	SPint int_dataset5[] = {69, 18, -94, -50, -43, -28, -108, -7, -106, 40};
	SPint int_dataset6[] = {11, -43, -92, -52, -65, -27, 109, 99, 6, 91};

	SPint int_dataset7[] = {52, -65, -90, 80, 54, -22, 117, 90, -8, -92};
	SPint int_dataset8[] = {-96, -11, -103, 42, -78, -71, -59, -90, 21, -47};
	SPint int_dataset9[] = {119, -31, -73, 90, 31, -80, 86, -100, 106, 13};

	SPlong long_dataset1[] = {-76, -17, 33, 109, -3, 42, -42, 62, 116, 34};
	SPlong long_dataset2[] = {-39, 47, -59, 4, 97, 59, 8, 11, -90, -10};
	SPlong long_dataset3[] = {-41, -79, -52, 61, 98, -66, 81, 75, -100, 113};

	SPlong long_dataset4[] = {-115, -76, 113, 57, 120, 59, 52, -89, 51, -118};
	SPlong long_dataset5[] = {69, 18, -94, -50, -43, -28, -108, -7, -106, 40};
	SPlong long_dataset6[] = {11, -43, -92, -52, -65, -27, 109, 99, 6, 91};

	SPlong long_dataset7[] = {52, -65, -90, 80, 54, -22, 117, 90, -8, -92};
	SPlong long_dataset8[] = {-96, -11, -103, 42, -78, -71, -59, -90, 21, -47};
	SPlong long_dataset9[] = {119, -31, -73, 90, 31, -80, 86, -100, 106, 13};

    SPfloat float_dataset1[] = {1.4, 2.43, 954.23, 1.32, 69.423};
	SPfloat float_dataset2[] = {96.8945f, 965.f, 1667.f};
	SPfloat float_dataset3[] = {56.35435f, .5652f, 182.3E+12f, 45.34};
	SPfloat float_dataset4[] = {86.43, 1.4171f, .866, .5838, 182.34f};
	SPfloat float_dataset5[] = {6878.34, 8128, .344, 969.23, 7.423, 10.444};
	SPfloat float_dataset6[] = {135.348f, 4527.2434f, 45.123, 923.34f};

    SPdouble double_dataset1[] = {1.4, 2.43, 954.23, 1.32, 69.423};
	SPdouble double_dataset2[] = {96.8945, 965.f, 1667.f};
	SPdouble double_dataset3[] = {56.35435, .5652, 182.3E+12, 45.34};
	SPdouble double_dataset4[] = {86.43, 1.4171, .866, .5838, 182.34};
	SPdouble double_dataset5[] = {6878.34, 8128, .344, 969.23, 7.423, 10.444};
    SPdouble double_dataset6[] = {135.348f, 4527.2434f, 45.123, 923.34f};

	const SPchar* str_dataset1[] = {"a4353", "234b", "c453", "fhujsd", "bfbf"};
	const SPchar* str_dataset2[] = {"e498nefsj", "fhf", "ggfshg", "fghijfk"};
	const SPchar* str_dataset3[] = {"g34jnf", "hfujs", "ijgdsfs", "dfhjjfd", "djfjnfd"};

	const SPchar* str_dataset4[] = {"jjfdjj", "kdofk", "lmnfd", "fkdj"};
	const SPchar* str_dataset5[] = {"lorem", "ipsum", "dolor", "sit", "amet"};
	const SPchar* str_dataset6[] = {"pkfdjk", "qkdfkdmf", "rdnfmdfn"};

	const SPchar* str_dataset7[] = {"skmgmf", "fjndmfndt", "dfkmkdfmu", "djfndf"};
	const SPchar* str_dataset8[] = {"abc", "def", "ghi", "jklmno", "pqrst", "uvwxyz"};
	const SPchar* str_dataset9[] = {"y", "z"};
	const SPchar* str_dataset10[] = {"1234", "567890", "12345", "678910", "2345", "347858"};

    MT3_tree subtree_1 = mt3_AllocTree();
    mt3_InsertByte(&subtree_1, "byte", -126);
	mt3_InsertShort(&subtree_1, "short", -750);
    mt3_InsertInt(&subtree_1, "int", -1667);
    mt3_InsertLong(&subtree_1, "long", 123456789000);
    mt3_InsertFloat(&subtree_1, "float", 3.14159265f);
    mt3_InsertDouble(&subtree_1, "double", 1.742734875845);
    mt3_InsertString(&subtree_1, "string", "motti was here");

    MT3_tree subtree_2 = mt3_AllocTree();
    mt3_InsertByte(&subtree_2, "byte", -128);
	mt3_InsertShort(&subtree_2, "short", -750);
    mt3_InsertInt(&subtree_2, "int", -1667);
    mt3_InsertLong(&subtree_2, "long", 123456789000);
    mt3_InsertFloat(&subtree_2, "float", 3.14159265f);
    mt3_InsertDouble(&subtree_2, "double", 1.742734875845);
    mt3_InsertString(&subtree_2, "string", "fjiaw was here");

    MT3_tree subtree_3 = mt3_AllocTree();
    mt3_InsertByte(&subtree_3, "byte", -128);
	mt3_InsertShort(&subtree_3, "short", -750);
    mt3_InsertInt(&subtree_3, "int", -1667);
    mt3_InsertLong(&subtree_3, "long", 123456789000);
    mt3_InsertFloat(&subtree_3, "float", 3.14159265f);
    mt3_InsertDouble(&subtree_3, "double", 1.742734875845);
    mt3_InsertString(&subtree_3, "string", "thommy was here");

    MT3_tree subtree_4 = mt3_AllocTree();
    mt3_InsertByte(&subtree_4, "byte", -128);
	mt3_InsertShort(&subtree_4, "short", -750);
    mt3_InsertInt(&subtree_4, "int", -1667);
    mt3_InsertLong(&subtree_4, "long", 123456789000);
    mt3_InsertFloat(&subtree_4, "float", 3.14159265f);
    mt3_InsertDouble(&subtree_4, "double", 1.742734875845);
    mt3_InsertString(&subtree_4, "string", "betelgus was here");

	MT3_tree tree = NULL;
	/*
	mt3_InsertTree(&tree, "subtree_123", subtree_1);
	mt3_InsertTree(&tree, "subtree_223", subtree_2);
	mt3_InsertTree(&tree, "subtree_323", subtree_3);
	mt3_InsertTree(&tree, "subtree_423", subtree_4);
    */
	
	mt3_InsertByte(&tree, "byte", -128);
	mt3_InsertShort(&tree, "short", -348);
    mt3_InsertInt(&tree, "int", -9384);
    mt3_InsertLong(&tree, "long", 39489384);
    mt3_InsertFloat(&tree, "float", 1.34535f);
    mt3_InsertDouble(&tree, "double", 1.988E+30);
    mt3_InsertString(&tree, "string", "hidegion was here");

    // plain arrays..
    mt3_InsertByteArray(&tree, "byte_array", sizeof(byte_dataset1) / sizeof(SPbyte), byte_dataset1);
    mt3_InsertShortArray(&tree, "short_array", sizeof(short_dataset1) / sizeof(SPshort), short_dataset1);
    mt3_InsertIntArray(&tree, "int_array", sizeof(int_dataset1) / sizeof(SPint), int_dataset1);
    mt3_InsertLongArray(&tree, "long_array", sizeof(long_dataset1) / sizeof(SPlong),long_dataset1);
    mt3_InsertFloatArray(&tree, "float_array", sizeof(float_dataset1) / sizeof(SPfloat), float_dataset1);
    mt3_InsertDoubleArray(&tree, "double_array", sizeof(double_dataset1) / sizeof(SPdouble), double_dataset1);
    mt3_InsertStringArray(&tree, "string_array", sizeof(str_dataset1) / sizeof(const SPchar*), str_dataset1);
	
    // multi byte arrays..
    MT3_array multi_type_array_1 = mt3_AllocArray();
    MT3_array multi_type_array_2 = NULL;
    MT3_array multi_type_array_3 = mt3_AllocArray();

/*
    mt3_ArrayInsertByteArray(&multi_type_array_1, sizeof(byte_dataset1) / sizeof(SPbyte),byte_dataset1);
    mt3_ArrayInsertByteArray(&multi_type_array_1, sizeof(byte_dataset2) / sizeof(SPbyte), byte_dataset2);
    mt3_ArrayInsertByteArray(&multi_type_array_1, sizeof(byte_dataset3) / sizeof(SPbyte), byte_dataset3);
    mt3_ArrayInsertByteArray(&multi_type_array_2, sizeof(byte_dataset4) / sizeof(SPbyte), byte_dataset4);
    mt3_ArrayInsertByteArray(&multi_type_array_2, sizeof(byte_dataset5) / sizeof(SPbyte), byte_dataset5);
    mt3_ArrayInsertByteArray(&multi_type_array_2, sizeof(byte_dataset6) / sizeof(SPbyte), byte_dataset6);
    mt3_ArrayInsertByteArray(&multi_type_array_3, sizeof(byte_dataset7) / sizeof(SPbyte), byte_dataset7);
    mt3_ArrayInsertByteArray(&multi_type_array_3, sizeof(byte_dataset8) / sizeof(SPbyte), byte_dataset8);
    mt3_ArrayInsertByteArray(&multi_type_array_3, sizeof(byte_dataset9) / sizeof(SPbyte), byte_dataset9);
*/

/*
    mt3_ArrayInsertShortArray(&multi_type_array_1, sizeof(short_dataset1) / sizeof(SPshort), short_dataset1);
    mt3_ArrayInsertShortArray(&multi_type_array_1, sizeof(short_dataset2) / sizeof(SPshort), short_dataset2);
    mt3_ArrayInsertShortArray(&multi_type_array_1, sizeof(short_dataset3) / sizeof(SPshort), short_dataset3);
    mt3_ArrayInsertShortArray(&multi_type_array_2, sizeof(short_dataset4) / sizeof(SPshort), short_dataset4);
    mt3_ArrayInsertShortArray(&multi_type_array_2, sizeof(short_dataset5) / sizeof(SPshort), short_dataset5);
    mt3_ArrayInsertShortArray(&multi_type_array_2, sizeof(short_dataset6) / sizeof(SPshort), short_dataset6);
    mt3_ArrayInsertShortArray(&multi_type_array_3, sizeof(short_dataset7) / sizeof(SPshort), short_dataset7);
    mt3_ArrayInsertShortArray(&multi_type_array_3, sizeof(short_dataset8) / sizeof(SPshort), short_dataset8);
    mt3_ArrayInsertShortArray(&multi_type_array_3, sizeof(short_dataset9) / sizeof(SPshort), short_dataset9);
*/
/*
    mt3_ArrayInsertIntArray(&multi_type_array_1, sizeof(int_dataset1) / sizeof(SPint), int_dataset1);
    mt3_ArrayInsertIntArray(&multi_type_array_1, sizeof(int_dataset2) / sizeof(SPint), int_dataset2);
    mt3_ArrayInsertIntArray(&multi_type_array_1, sizeof(int_dataset3) / sizeof(SPint), int_dataset3);
    mt3_ArrayInsertIntArray(&multi_type_array_2, sizeof(int_dataset4) / sizeof(SPint), int_dataset4);
    mt3_ArrayInsertIntArray(&multi_type_array_2, sizeof(int_dataset5) / sizeof(SPint), int_dataset5);
    mt3_ArrayInsertIntArray(&multi_type_array_2, sizeof(int_dataset6) / sizeof(SPint), int_dataset6);
    mt3_ArrayInsertIntArray(&multi_type_array_3, sizeof(int_dataset7) / sizeof(SPint), int_dataset7);
    mt3_ArrayInsertIntArray(&multi_type_array_3, sizeof(int_dataset8) / sizeof(SPint), int_dataset8);
    mt3_ArrayInsertIntArray(&multi_type_array_3, sizeof(int_dataset9) / sizeof(SPint), int_dataset9);
*/
/*
    mt3_ArrayInsertLongArray(&multi_type_array_1, sizeof(long_dataset1) / sizeof(SPlong), long_dataset1);
    mt3_ArrayInsertLongArray(&multi_type_array_1, sizeof(long_dataset2) / sizeof(SPlong), long_dataset2);
    mt3_ArrayInsertLongArray(&multi_type_array_1, sizeof(long_dataset3) / sizeof(SPlong), long_dataset3);
    mt3_ArrayInsertLongArray(&multi_type_array_2, sizeof(long_dataset4) / sizeof(SPlong), long_dataset4);
    mt3_ArrayInsertLongArray(&multi_type_array_2, sizeof(long_dataset5) / sizeof(SPlong), long_dataset5);
    mt3_ArrayInsertLongArray(&multi_type_array_2, sizeof(long_dataset6) / sizeof(SPlong), long_dataset6);
    mt3_ArrayInsertLongArray(&multi_type_array_3, sizeof(long_dataset7) / sizeof(SPlong), long_dataset7);
    mt3_ArrayInsertLongArray(&multi_type_array_3, sizeof(long_dataset8) / sizeof(SPlong), long_dataset8);
    mt3_ArrayInsertLongArray(&multi_type_array_3, sizeof(long_dataset9) / sizeof(SPlong), long_dataset9);
*/
/*
    mt3_ArrayInsertFloatArray(&multi_type_array_1, sizeof(float_dataset1) / sizeof(SPfloat), float_dataset1);
    mt3_ArrayInsertFloatArray(&multi_type_array_1, sizeof(float_dataset2) / sizeof(SPfloat), float_dataset2);
    mt3_ArrayInsertFloatArray(&multi_type_array_2, sizeof(float_dataset3) / sizeof(SPfloat), float_dataset3);
    mt3_ArrayInsertFloatArray(&multi_type_array_2, sizeof(float_dataset4) / sizeof(SPfloat), float_dataset4);
    mt3_ArrayInsertFloatArray(&multi_type_array_3, sizeof(float_dataset5) / sizeof(SPfloat), float_dataset5);
    mt3_ArrayInsertFloatArray(&multi_type_array_3, sizeof(float_dataset6) / sizeof(SPfloat), float_dataset6);
*/
/*
    mt3_ArrayInsertDoubleArray(&multi_type_array_1, sizeof(double_dataset1) / sizeof(SPdouble), double_dataset1);
    mt3_ArrayInsertDoubleArray(&multi_type_array_1, sizeof(double_dataset2) / sizeof(SPdouble), double_dataset2);
    mt3_ArrayInsertDoubleArray(&multi_type_array_2, sizeof(double_dataset3) / sizeof(SPdouble), double_dataset3);
    mt3_ArrayInsertDoubleArray(&multi_type_array_2, sizeof(double_dataset4) / sizeof(SPdouble), double_dataset4);
    mt3_ArrayInsertDoubleArray(&multi_type_array_3, sizeof(double_dataset5) / sizeof(SPdouble), double_dataset5);
    mt3_ArrayInsertDoubleArray(&multi_type_array_3, sizeof(double_dataset6) / sizeof(SPdouble), double_dataset6);
*/

    mt3_ArrayInsertStringArray(&multi_type_array_1, sizeof(str_dataset1) / sizeof(const SPchar*), str_dataset1);
    mt3_ArrayInsertStringArray(&multi_type_array_1, sizeof(str_dataset2) / sizeof(const SPchar*), str_dataset2);
    mt3_ArrayInsertStringArray(&multi_type_array_1, sizeof(str_dataset3) / sizeof(const SPchar*), str_dataset3);
    mt3_ArrayInsertStringArray(&multi_type_array_2, sizeof(str_dataset4) / sizeof(const SPchar*), str_dataset4);
    mt3_ArrayInsertStringArray(&multi_type_array_2, sizeof(str_dataset5) / sizeof(const SPchar*), str_dataset5);
    mt3_ArrayInsertStringArray(&multi_type_array_2, sizeof(str_dataset6) / sizeof(const SPchar*), str_dataset6);
    mt3_ArrayInsertStringArray(&multi_type_array_3, sizeof(str_dataset7) / sizeof(const SPchar*), str_dataset7);
    mt3_ArrayInsertStringArray(&multi_type_array_3, sizeof(str_dataset8) / sizeof(const SPchar*), str_dataset8);
    mt3_ArrayInsertStringArray(&multi_type_array_3, sizeof(str_dataset9) / sizeof(const SPchar*), str_dataset9);
    mt3_ArrayInsertStringArray(&multi_type_array_3, sizeof(str_dataset10) / sizeof(const SPchar*), str_dataset10);
/*
    mt3_ArrayInsertTree(&multi_type_array_1, subtree_1);
    mt3_ArrayInsertTree(&multi_type_array_1, subtree_2);
    mt3_ArrayInsertTree(&multi_type_array_1, subtree_3);
    mt3_ArrayInsertTree(&multi_type_array_1, subtree_4);
*/
    //mt3_InsertArray(&tree, "multi_type_array_1", multi_type_array_1);

    // multi multi byte arrays..
    MT3_array multi_multi_type_array = NULL;
    mt3_ArrayInsertArray(&multi_multi_type_array, multi_type_array_1);
    //mt3_InsertArray(&tree, "multi_multi_type_array", multi_multi_type_array);

    MT3_array multi_multi_multi_type_array = mt3_AllocArray();
    mt3_ArrayInsertArray(&multi_multi_multi_type_array, multi_multi_type_array);
    mt3_InsertArray(&tree, "multi_multi_multi_type_array", multi_multi_multi_type_array);

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
    mt3_FreeTree(&subtree_1);
    mt3_FreeTree(&subtree_2);
    mt3_FreeTree(&subtree_3);
    mt3_FreeTree(&subtree_4);
    mt3_FreeTree(&output);
    mt3_FreeArray(&multi_type_array_1);
    mt3_FreeArray(&multi_type_array_2);
    mt3_FreeArray(&multi_type_array_3);
    mt3_FreeArray(&multi_multi_type_array);
    mt3_FreeArray(&multi_multi_multi_type_array);
}
int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);

	/*
	SP_TEST_ADD(test_insertion);
	SP_TEST_ADD(test_null_tree);
	SP_TEST_ADD(test_insert_empty_tree);
	SP_TEST_ADD(test_sub_tree);
	SP_TEST_ADD(test_tree_random_integers);
	SP_TEST_ADD(test_random_index);
    */

    SP_TEST_ADD(big_test);
	spTestRunAll();
	spTestTerminate();
	return 0;
}