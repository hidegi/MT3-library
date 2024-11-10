#include <SP/test/unit.h>
#include "rbt.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>


unsigned long long values[][20] =
{
	{554, 865, 140, 9, 893, 859, 616, 146, 559, 166, 	852, 525, 538, 241, 540, 994, 699, 730, 701, 548}
};
/*
 *	deletion rule:
 *
 *	- if to be deleted node has no children, replacement is NULL..
 *	- if to be deleted node has one child, replacement is child..
 *	- if to be deleted node has two children:
 *	
 */

bool containsNumber(int n, const int* array, int length)
{
    for(int i = 0; i < length; i++)
        if(array[i] == n)
            return true;
    return false;
}

/*
 *  whenever there is triangle, rotate right-left or left-right (2 rotations)..
 *  whenever there is skew, rotate left or right (1 rotation)..
 *  rotate always around the parent of the newly added node..
 */


void test_tree_insert()
{
    //41 13 87 96 95 25 31 54 43 19 1 22 77 99 65 64 73 4 55 93
    //2 65 52 6 38 34 68 40 85 72 70 39 16 24 54 26 28 89 45 97
	
	//srand(time(NULL));
    
	/*
	for(int i = 19; i >= 1; i--)
	{
		//int value = rand() % 1000 + 1;
		nodeAdd_RBT(n, &n, i);
	}
	*/
	/*
	 *	417465280
	 *	120
	 *	121
	 *	2283126203
	 *	1369249285
	 *	632064625
	 *	915915944
	 *	872506760
	 *	872506761
	 *	872506762
	 *	872506763
	 *	4109031586
	 *	510480399
	 */
	Node* n = nodeAlloc(417465280);
	nodeAdd_RBT(n, &n, 120);
	nodeAdd_RBT(n, &n, 121);
	nodeAdd_RBT(n, &n, 2283126203);
	nodeAdd_RBT(n, &n, 1369249285);
	nodeAdd_RBT(n, &n, 632064625);
	nodeAdd_RBT(n, &n, 915915944);
	nodeAdd_RBT(n, &n, 872506760);
	nodeAdd_RBT(n, &n, 872506761);
	nodeAdd_RBT(n, &n, 872506762);
	nodeAdd_RBT(n, &n, 872506763);
	nodeAdd_RBT(n, &n, 4109031586);
	nodeAdd_RBT(n, &n, 510480399);
	printf("\n");

    nodePrint(n);
    nodeFree(n);
}

void test_find()
{
	Node* n = nodeAlloc(417465280);
	nodeAdd_RBT(n, &n, 120);
	nodeAdd_RBT(n, &n, 121);
	nodeAdd_RBT(n, &n, 24);
	nodeAdd_RBT(n, &n, 642);
	//nodeAdd_RBT(n, &n, 24);
	//nodeAdd_RBT(n, &n, 642);
	nodeAdd_RBT(n, &n, 135);
	nodeAdd_RBT(n, &n, 125);
	nodeAdd_RBT(n, &n, 16);
	nodeAdd_RBT(n, &n, 98);
	printf("\n");
    /*
	Node* toFind = nodeFind(n, 121);
	printf("node data: %lld\n", toFind->data);
	SP_ASSERT_NOT_NULL(toFind);
	*/
    nodePrint(n);
    nodeFree(n);
}


constexpr int array_ex_1[] = {13, 8, 17, 1, 11, 15, 25, 6, 22, 27};
constexpr int array_ex_2[] = {7, 3, 18, 10, 22, 8, 11, 26};
constexpr int array_ex_3[] = {5, 2, 8, 1, 4, 7, 9};
constexpr int array_big_test[] = {123, 886, 93, 808, 220, 768, 527, 811, 946, 219, 546, 926, 840, 332, 200, 236, 489, 533, 999, 727, 279, 585, 485, 199, 64, 105, 280, 491, 466, 788, 1, 449, 803, 404, 492, 846, 374, 226, 603, 160, 644, 868, 740, 853, 27, 77, 337, 752, 773, 549, 641, 582, 454, 635, 719, 506, 857, 581, 51, 844, 207, 233, 564, 795, 94, 314, 530, 37, 542, 518, 29, 472, 483, 428, 142, 917, 242, 303, 708, 559, 155, 938, 264, 852, 642, 895, 298, 331, 550, 933, 942, 65, 631, 248, 186, 410, 744, 357, 859, 206};
#
void test_rbt_delete_1()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	/*
	printf("before:\n");
	nodePrint(n);
	
	nodeDelete_RBT(&n, 6);
		
	printf("\n\nafter:\n");
	nodePrint(n);
	*/
	
	SP_ASSERT_TRUE(
	n->data == 13 && 
	n->minor->data == 8 && 
	n->major->data == 17 &&
	n->major->minor->data == 15 &&
	n->minor->major->data == 11);
    nodeFree(n);
}

void test_rbt_delete_2()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	//printf("before:\n");
	//nodePrint(n);
	
	nodeDelete_RBT(&n, 1);
	//printf("\n\nafter:\n");
	//nodePrint(n);
	SP_ASSERT_NOT_NULL(n);
	SP_ASSERT_TRUE(
	n->data == 13 && 
	n->minor->data == 8 && 
	n->major->data == 17 && 
	n->minor->minor->data == 6 && 
	n->minor->major->data == 11
	);
	
	
    nodeFree(n);
}

void test_rbt_delete_3()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	
	
	//printf("\n\nafter:\n");
	//nodePrint(n);
	
	nodeDelete_RBT(&n, 17);
	printf("after:\n");
	nodePrint(n);
	
	SP_ASSERT_TRUE_WITH_ACTION(
	n->data == 13 &&
	n->major->data == 22 &&
	n->minor->data == 8 &&
	n->major->major->data == 25 &&
	n->major->minor->data == 15, nodeFree(n)
	);
    nodeFree(n);
}

void test_bst_delete_3()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	
	
	//printf("\n\nafter:\n");
	//nodePrint(n);
	Node* r;
	auto xw = nodeDelete_BST(&n, 17, &r);
	
	SP_ASSERT_NOT_NULL_WITH_ACTION(n, nodeFree(n));
	printf("after:\n");
	nodePrint(n);
	
	Node* x = xw.first;
	Node* w = xw.second;
	
	SP_ASSERT_NULL_WITH_ACTION(x, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(w, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(27, w->data, nodeFree(n));
	
    nodeFree(n);
}

void test_bst_delete_4()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	Node* r;
	auto xw = nodeDelete_BST(&n, 25, &r);
	Node* x = xw.first;
	Node* w = xw.second;
	SP_ASSERT_NOT_NULL_WITH_ACTION(n, nodeFree(n));
	nodePrint(n);
	SP_ASSERT_NULL_WITH_ACTION(x, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(w, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(22, w->data, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(w->red, nodeFree(n));
	
    nodeFree(n);
}

void test_rbt_delete_4()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	
	nodeDelete_RBT(&n, 25);
	
	nodePrint(n);
	SP_ASSERT_TRUE_WITH_ACTION(
	n->data == 13 &&
	n->major->data == 17 &&
	n->minor->data == 8, nodeFree(n));
	
	SP_ASSERT_TRUE_WITH_ACTION(
	n->major->major->data == 27 &&
	n->major->minor->data == 15,
	nodeFree(n));
	
	SP_ASSERT_TRUE_WITH_ACTION(
	!n->major->major->major && 
	n->major->major->minor->data == 22,
	nodeFree(n));
    nodeFree(n);
}

void test_rbt_delete_5()
{
	Node* n = nodeAlloc(array_ex_2[0]);
	for(int i = 1; i < sizeof(array_ex_2) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_2[i]);
	}
	
	nodeDelete_RBT(&n, 18);
	SP_ASSERT_TRUE(
	n->data == 7 &&
	n->major->data == 22 &&
	n->minor->data == 3
	);
	
	
	SP_ASSERT_TRUE(
	n->major->major->data == 26 &&
	n->major->minor->data == 10);
	
	SP_ASSERT_TRUE(
	n->major->minor->minor->data == 8 &&
	n->major->minor->major->data == 11);
	
	SP_ASSERT_TRUE(
	n->major->major->major == NULL &&
	n->major->major->minor == NULL
	);
	
    nodeFree(n);
}

void test_rbt_delete_6()
{
	int array[] = {5, 2, 8, 1, 4, 7, 9, 3, 0};
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	nodePrint(n);
	nodeDelete_RBT(&n, 4);
	/*
	SP_ASSERT_TRUE(
	n->data == 5 &&
	n->major->data == 8 &&
	n->minor->data == 4
	);
	
	SP_ASSERT_TRUE(
	n->minor->minor->data == 1 &&
	!n->minor->major
	);
	
	SP_ASSERT_TRUE(
	n->major->minor->data == 7 &&
	n->major->major->data == 9
	);
	*/
    nodeFree(n);
}

void test_bst_delete_7()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	
	Node* r;
	auto xw = nodeDelete_BST(&n, 13, &r);
	Node* x = xw.first;
	Node* w = xw.second;
	
	SP_ASSERT_NULL_WITH_ACTION(x, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(w, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(r, nodeFree(n));
	
	SP_ASSERT_TRUE_WITH_ACTION(r->data == 15, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(w->data == 25, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(!w->red, nodeFree(n));
	
    nodeFree(n);
}

void test_rbt_delete_7()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	nodeDelete_RBT(&n, 13);
	nodePrint(n);
	SP_ASSERT_TRUE(
	n->data == 15 &&
	n->major->data == 25 &&
	n->minor->data == 8
	);
	
	Node* testBranch = n->major;
	SP_ASSERT_TRUE(
	testBranch->major->data == 27 &&
	testBranch->minor->data == 17
	);
	
	testBranch = testBranch->major;
	SP_ASSERT_TRUE(
	testBranch->major == NULL &&
	testBranch->minor == NULL
	);
	
	testBranch = testBranch->parent->minor;
	SP_ASSERT_TRUE(
	testBranch->major->data == 22 &&
	testBranch->minor == NULL
	);
	
    nodeFree(n);
}

void test_bst_delete_8()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	printf("before:\n");
	nodePrint(n);
	printf("after:\n");
	Node* r;
	auto xw = nodeDelete_BST(&n, 8, &r);
	nodePrint(n);
	Node* x = xw.first;
	Node* w = xw.second;
	SP_ASSERT_NOT_NULL_WITH_ACTION(x, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(w, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(x->red, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(w->red, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(11, x->data, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(1, w->data, nodeFree(n));
	
    nodeFree(n);
}

void test_rbt_delete_8()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	nodeDelete_RBT(&n, 8);
	nodePrint(n);
	Node* testBranch = n;
	SP_ASSERT_TRUE_WITH_ACTION(
		testBranch->data == 13 &&
		testBranch->major->data == 17 &&
		(testBranch->minor->data == 6 || testBranch->minor->data == 11),
		nodeFree(n)
	);
	
	testBranch = n->minor;
	SP_ASSERT_NOT_NULL_WITH_ACTION(testBranch->major, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(testBranch->minor, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(
		testBranch->major->data == 11 &&
		testBranch->minor->data == 1,
		nodeFree(n)
	);
	
	testBranch = n->major;
	SP_ASSERT_TRUE_WITH_ACTION(
		testBranch->major->data == 25 &&
		testBranch->minor->data == 15,
		nodeFree(n)
	);
	
    nodeFree(n);
}

void test_bst_delete_9()
{
	Node* n = nodeAlloc(array_ex_2[0]);
	for(int i = 1; i < sizeof(array_ex_2) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_2[i]);
	}
	
	Node* r;
	auto xw = nodeDelete_BST(&n, 3, &r);
	Node* x = xw.first;
	Node* w = xw.second;
	SP_ASSERT_TRUE_WITH_ACTION(n, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(n->data == 7, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(!x, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(w, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(w->data == 18, nodeFree(n));
	SP_ASSERT_NULL_WITH_ACTION(r, nodeFree(n));
    nodeFree(n);
}

void test_rbt_delete_9()
{
	Node* n = nodeAlloc(array_ex_2[0]);
	for(int i = 1; i < sizeof(array_ex_2) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_2[i]);
	}
	nodeDelete_RBT(&n, 3);
	SP_ASSERT_TRUE_WITH_ACTION(n, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(n->data == 18, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(n->major, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(n->minor, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(10, n->minor->data, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(22, n->major->data, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(!n->major->red, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(n->minor->red, nodeFree(n));
    nodeFree(n);
}

void test_rbt_delete_10()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	
	nodeDelete_RBT(&n, 11);
	SP_ASSERT_TRUE_WITH_ACTION(n, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(n->data == 13, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(n->major, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(n->minor, nodeFree(n));
	
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(6, n->minor->data, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(17, n->major->data, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(17, n->major->data, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(n->major->red, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(n->minor->red, nodeFree(n));
	nodePrint(n);
    nodeFree(n);
}

void test_bst_delete_11()
{
	int array[] = {13, 8, 17, 1, 11, 15, 25, 6, 22, 27, 28};
	
	Node* n = nodeAlloc(array[0]);
	for(int i = 1; i < sizeof(array) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array[i]);
	}
	nodePrint(n);
	
	Node* r;
	auto xw = nodeDelete_BST(&n, 27, &r);
	Node* x = xw.first;
	Node* w = xw.second;
	nodePrint(n);
	SP_ASSERT_TRUE_WITH_ACTION(n, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(x, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(w, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(22, w->data, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(28, x->data, nodeFree(n));
	
	
    nodeFree(n);
}

void test_rbt_delete_11()
{
	int array[] = {13, 8, 17, 1, 11, 15, 25, 6, 22, 27, 28};
	
	Node* n = nodeAlloc(array[0]);
	for(int i = 1; i < sizeof(array) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array[i]);
	}
	nodePrint(n);
	nodeDelete_RBT(&n, 27);
	nodePrint(n);
    nodeFree(n);
}

void test_rbt_delete_big_test()
{
	
	Node* n = nodeAlloc(array_big_test[0]);
	const SPsize length = sizeof(array_big_test) / sizeof(int);
	for(int i = 1; i < length; i++)
	{
		nodeAdd_RBT(n, &n, array_big_test[i]);
	}
	nodePrint(n);
	/*
	for(int i = length - 1; i >= 1; i--)
	{
		SP_DEBUG("deleting %d", array_big_test[i]);
	}
	*/
	nodeDelete_RBT(&n, 219);
	nodePrint(n);
    nodeFree(n);
}

void test_bst_delete()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	printf("before:\n");
	nodePrint(n);
	Node* r;
	nodeDelete_BST(&n, 25, &r);
	printf("\n\nafter:\n");
	nodePrint(n);
    nodeFree(n);
}

void test_bst_delete_no_children_no_parent()
{
	// BST delete case 1
	Node* n = nodeAlloc(5);
	
	Node* r;
	auto xw = nodeDelete_BST(&n, 5, &r);
	Node* x = xw.first;
	Node* w = xw.second;
	
	SP_ASSERT_NULL_WITH_ACTION(x, nodeFree(n));
	SP_ASSERT_NULL_WITH_ACTION(w, nodeFree(n));
	SP_ASSERT_NULL_WITH_ACTION(n, nodeFree(n));
    nodeFree(n);
}

void test_bst_delete_one_child_no_parent()
{
	// BST delete case 2
	Node* n = nodeAlloc(5);
	nodeAdd_RBT(n, &n, 6);
	
	Node* r;
	auto xw = nodeDelete_BST(&n, 5, &r);
	Node* x = xw.first;
	Node* w = xw.second;
	SP_ASSERT_NULL_WITH_ACTION(x, nodeFree(n));
	SP_ASSERT_NULL_WITH_ACTION(w, nodeFree(n));
	
	SP_ASSERT_TRUE_WITH_ACTION(n->data == 6, nodeFree(n));
    nodeFree(n);
}

void test_bst_delete_no_children_and_parent()
{
	// BST delete case 1
	Node* n = nodeAlloc(5);
	nodeAdd_RBT(n, &n, 6);
	
	Node* r;
	auto xw = nodeDelete_BST(&n, 6, &r);
	Node* x = xw.first;
	Node* w = xw.second;
	
	SP_ASSERT_NULL_WITH_ACTION(x, nodeFree(n));
	SP_ASSERT_NULL_WITH_ACTION(w, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(6, n->data, nodeFree(n));
    nodeFree(n);
}

void test_bst_delete_one_child_and_parent()
{
	// BST delete case 2
	Node* n = nodeAlloc(5);
	nodeAdd_RBT(n, &n, 3);
	nodeAdd_RBT(n, &n, 7);
	nodeAdd_RBT(n, &n, 1);
	
	Node* r;
	auto xw = nodeDelete_BST(&n, 3, &r);
	Node* x = xw.first;
	
	SP_ASSERT_NOT_NULL(x);
	SP_ASSERT_TRUE(x->data == 1);
	SP_ASSERT_NOT_NULL(x->parent);
	SP_ASSERT_NOT_NULL(x->parent->major);
	SP_ASSERT_INTEGER_EQUAL(7, x->parent->major->data);
    nodeFree(n);
}

void test_bst_delete_two_children_major_minimum()
{
	// BST delete case 3.1
	Node* n = nodeAlloc(5);
	
	nodeAdd_RBT(n, &n, 3);
	nodeAdd_RBT(n, &n, 7);
	nodeAdd_RBT(n, &n, 1);
	nodeAdd_RBT(n, &n, 4);
	nodeAdd_RBT(n, &n, 8);
	
	Node* r;
	auto xw = nodeDelete_BST(&n, 5, &r);
	Node* x = xw.first;
	

	SP_ASSERT_NOT_NULL_WITH_ACTION(n, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(x, nodeFree(n));
	SP_ASSERT_TRUE(n->data == 7);
	SP_ASSERT_INTEGER_EQUAL(8, x->data);
    nodeFree(n);
}

void test_bst_delete_two_children_major_minor_minimum()
{
	// BST delete case 3.2
	Node* n = nodeAlloc(15);
	
	nodeAdd_RBT(n, &n, 5);
	nodeAdd_RBT(n, &n, 23);
	nodeAdd_RBT(n, &n, 4);
	nodeAdd_RBT(n, &n, 13);
	nodeAdd_RBT(n, &n, 17);
	nodeAdd_RBT(n, &n, 28);
	nodeAdd_RBT(n, &n, 19);
	printf("before:\n");
	nodePrint(n);
	
	Node* r;
	auto xw = nodeDelete_BST(&n, 15, &r);
	Node* x = xw.first;
	
	SP_ASSERT_TRUE(n->data == 17);
	SP_ASSERT_NOT_NULL(x);
	SP_ASSERT_TRUE(x->data == 19);
	
	printf("after:\n");
	nodePrint(n);
	
    nodeFree(n);
}

void test_bst_delete_two_children_x_minimum()
{
	// BST delete case 3.3
	Node* n = nodeAlloc(50);
	
	nodeAdd_RBT(n, &n, 32);
	nodeAdd_RBT(n, &n, 75);
	nodeAdd_RBT(n, &n, 15);
	nodeAdd_RBT(n, &n, 44);
	nodeAdd_RBT(n, &n, 55);
	nodeAdd_RBT(n, &n, 99);
	nodeAdd_RBT(n, &n, 10);
	nodeAdd_RBT(n, &n, 19);
	nodeAdd_RBT(n, &n, 51);
	nodeAdd_RBT(n, &n, 64);
	nodeAdd_RBT(n, &n, 54);
	printf("before:\n");
	nodePrint(n);
	Node* r;
	auto xw = nodeDelete_BST(&n, 50, &r);
	Node* x = xw.first;
	
	printf("after:\n");
	nodePrint(n);
	
    nodeFree(n);
}

void test_bst_delete_two_children_no_parent()
{
	Node* n = nodeAlloc(5);
	nodeAdd_RBT(n, &n, 3);
	nodeAdd_RBT(n, &n, 7);
	nodeAdd_RBT(n, &n, 1);
	nodeAdd_RBT(n, &n, 4);
	nodeAdd_RBT(n, &n, 6);
	nodeAdd_RBT(n, &n, 8);
	Node* r;
	nodeDelete_BST(&n, 5, &r);
	
	SP_ASSERT_TRUE(n->data == 6);
	
	SP_ASSERT_NOT_NULL(n->major);
	Node* x = n->major->minor;
	SP_ASSERT_TRUE(x == NULL);
	
    nodeFree(n);
}

void test_bst_delete_1()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	Node* r;
	auto xw = nodeDelete_BST(&n, 8, &r);
	Node* x = xw.first;
	Node* w = xw.second;
	Node* testBranch = n;
	testBranch = n->major;
	
	SP_ASSERT_NULL_WITH_ACTION(x, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(w, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(1, w->data, nodeFree(n));
	
	SP_ASSERT_NOT_NULL_WITH_ACTION(n->minor, nodeFree(n));
	SP_ASSERT_NOT_NULL_WITH_ACTION(n->major, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(11, n->minor->data, nodeFree(n));
	SP_ASSERT_INTEGER_EQUAL_WITH_ACTION(17, n->major->data, nodeFree(n));
	nodePrint(n);
    nodeFree(n);
}


#define LENGTH 200
void test_rbt_delete_random()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dst(1,1000);

	Node* n = nodeAlloc(123);
	
	int* array = new int[LENGTH];
	array[0] = 123;
	SPsize length = LENGTH;
	printf("123");
	for(int i = 1; i < length; i++)
	{
	    int num = dst(rng);
	    while(containsNumber(num, array, LENGTH))
	        num = dst(rng);
		array[i] = num;
		printf(", %lld", array[i]);
		nodeAdd_RBT(n, &n, array[i]);
	}
	
	printf("\n");
	printf("before:\n");
    nodePrint(n);

	//nodeDelete_RBT(&n, 123);
	//nodeDelete_RBT(&n, 559);
	
	for(int i = length - 1; i >= 1; i--)
	{
		nodeDelete_RBT(&n, array[i]);
	}
	
	printf("\n\nafter:\n");
    nodeFree(n);
	delete[] array;
}

void test_bst_delete_random()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dst(1,1000);

	Node* n = nodeAlloc(123);
	
	int* array = new int[LENGTH];
	array[0] = 123;
	SPsize length = LENGTH;
	printf("123");
	for(int i = 1; i < length; i++)
	{
	    int num = dst(rng);
	    while(containsNumber(num, array, LENGTH))
	        num = dst(rng);
		array[i] = num;
		printf(", %lld", array[i]);
		nodeAdd_RBT(n, &n, array[i]);
	}
	
	printf("\n");
	printf("before:\n");
    nodePrint(n);

	//nodeDelete_RBT(&n, 123);
	Node* r;
	nodeDelete_BST(&n, 559, &r);
	
	for(int i = length - 1; i >= 1; i--)
	{
		auto xw = nodeDelete_BST(&n, array[i], &r);
		Node* x = xw.first;
		Node* w = xw.second;
		
		if(x && w)
		{
			SP_ASSERT_TRUE_WITH_ACTION(x->parent == w->parent, 
				{	
					nodeFree(n); 
					delete [] array;
				}
			);
		}
	}
	
	printf("\n\nafter:\n");
	nodePrint(n);
    nodeFree(n);
	delete[] array;
}

void test_another()
{
}

void test_that()
{
}

int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
    //SP_TEST_ADD(test_find);
	
	// fail 9, 10
	
	SP_TEST_ADD(test_bst_delete_9);
    SP_TEST_ADD(test_rbt_delete_9);
	
    SP_TEST_ADD(test_rbt_delete_1);
    SP_TEST_ADD(test_rbt_delete_2);
    SP_TEST_ADD(test_rbt_delete_3);
    SP_TEST_ADD(test_bst_delete_3);
    SP_TEST_ADD(test_bst_delete_4);
	SP_TEST_ADD(test_rbt_delete_4);
    SP_TEST_ADD(test_rbt_delete_5);
    SP_TEST_ADD(test_rbt_delete_6);
    SP_TEST_ADD(test_rbt_delete_7);
    SP_TEST_ADD(test_rbt_delete_8);
    SP_TEST_ADD(test_rbt_delete_10);
    SP_TEST_ADD(test_bst_delete_11);
    SP_TEST_ADD(test_rbt_delete_11);
	SP_TEST_ADD(test_bst_delete_7);
	SP_TEST_ADD(test_rbt_delete_7);
	
    //SP_TEST_ADD(test_bst_delete_random);
    SP_TEST_ADD(test_rbt_delete_random);
	
	
    SP_TEST_ADD(test_bst_delete_no_children_no_parent);
    SP_TEST_ADD(test_bst_delete_one_child_no_parent);
    SP_TEST_ADD(test_bst_delete_one_child_and_parent);
    SP_TEST_ADD(test_bst_delete_two_children_major_minimum);
    SP_TEST_ADD(test_bst_delete_two_children_major_minor_minimum);
	
    SP_TEST_ADD(test_bst_delete_two_children_x_minimum);
	
	
    //SP_TEST_ADD(test_bst_delete_1);

	spTestRunAll();
	spTestTerminate();
	return 0;
}