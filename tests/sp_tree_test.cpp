#include <SP/test/unit.h>
#include "rbt.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

unsigned long long values[][20] =
{
	{554, 865, 140, 9, 893, 859, 616, 146, 559, 166, 852, 525, 538, 241, 540, 994, 699, 730, 701, 548}
};

int calculateBlackDepth(Node* node) 
{
    if(!node) 
        return 1;

    int count = 0;
    if(!node->red) 
        count++;
	
    int leftDepth = calculateBlackDepth(node->major);
    int rightDepth = calculateBlackDepth(node->minor);
	
    return count + std::max(leftDepth, rightDepth);
}

bool verifyImpl_RBT(Node* rbt, int depth, const int reference)
{
	if(rbt)
	{
		if(rbt->red)
		{
			if(!rbt->parent)
			{
				SP_DEBUG("red root node");
				return false;
			}
			else
			{
				if(rbt->parent->red)
				{
					SP_DEBUG("red node with red parent");
					return false;
				}
			}
		}
		else
		{
			++depth;
		}
		
		if(rbt->major || rbt->minor)
		{
			bool a = true, b = true;
			if(rbt->major)
			{
				a = verifyImpl_RBT(rbt->major, depth, reference); 
			}
			if(rbt->minor)
			{
				b = verifyImpl_RBT(rbt->minor, depth, reference); 
			}
			return a && b;
		}
		else
		{
			return !rbt->red ? reference == depth : true;
		}
	}
	
	return true;
}
bool verifyRBT(Node* rbt)
{
	int depth = calculateBlackDepth(rbt) - 1;
	return verifyImpl_RBT(rbt, 0, depth);
}

bool containsNumber(int n, const int* array, int length)
{
    for(int i = 0; i < length; i++)
        if(array[i] == n)
            return true;
    return false;
}

constexpr int array_ex_1[] = {13, 8, 17, 1, 11, 15, 25, 6, 22, 27};
constexpr int array_ex_2[] = {7, 3, 18, 10, 22, 8, 11, 26};
constexpr int array_ex_3[] = {5, 2, 8, 1, 4, 7, 9};
constexpr int array_big_test[] = {123, 886, 93, 808, 220, 768, 527, 811, 946, 219, 546, 926, 840, 332, 200, 236, 489, 533, 999, 727, 279, 585, 485, 199, 64, 105, 280, 491, 466, 788, 1, 449, 803, 404, 492, 846, 374, 226, 603, 160, 644, 868, 740, 853, 27, 77, 337, 752, 773, 549, 641, 582, 454, 635, 719, 506, 857, 581, 51, 844, 207, 233, 564, 795, 94, 314, 530, 37, 542, 518, 29, 472, 483, 428, 142, 917, 242, 303, 708, 559, 155, 938, 264, 852, 642, 895, 298, 331, 550, 933, 942, 65, 631, 248, 186, 410, 744, 357, 859, 206};

constexpr int array_big_test_1[] = 
{
123, 666, 146, 572, 60, 255, 171, 236, 286, 74, 777, 372, 668, 44, 391, 548, 540, 869, 419, 328, 610, 62, 132, 320, 85, 281, 159, 186, 430, 152, 134, 724, 560, 842, 880, 477, 529, 169, 715, 516, 965, 470, 418, 40, 427, 693, 811, 652, 76, 101, 363, 49, 731, 721, 68, 597, 11, 685, 284, 109, 298, 549, 118, 367, 54, 230, 532, 308, 716, 735, 357, 420, 264, 649, 89, 857, 887, 7, 847, 776, 861, 849, 617, 83, 613, 596, 237, 561, 51, 67, 424, 86, 482, 585, 459, 437, 569, 997, 350, 246, 145, 769, 942, 165, 750, 577, 660, 46, 257, 771, 674, 645, 147, 9, 416, 3, 797, 88, 336, 533, 117, 486, 678, 19, 487, 104, 651, 746, 831, 288, 828, 940, 900, 550, 348, 852, 138, 547, 500, 149, 510, 481, 102, 24, 396, 75, 607, 916, 798, 509, 303, 506, 239, 566, 373, 700, 154, 840, 854, 14, 927, 962, 125, 20, 270, 402, 665, 414, 25, 435, 654, 722, 809, 555, 899, 743, 457, 148, 184, 753, 222, 658, 841, 81, 970, 64, 251, 266, 63, 208, 33, 614, 227, 542, 130, 851, 807, 768, 189, 442
};

constexpr int array_ex_rotation[] = {7, 493, 352, 244, 136, 499, 212, 482, 112, 167, 218, 495, 223, 181, 365, 353, 113, 264, 335, 265};
void test_rbt_delete_1()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}

	SP_ASSERT_TRUE(
	n->data == 13 && 
	n->minor->data == 8 && 
	n->major->data == 17 &&
	n->major->minor->data == 15 &&
	n->minor->major->data == 11);
	
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
    nodeFree(n);
}

void test_rbt_delete_2()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	
	nodeDelete_RBT(&n, 1);
	SP_ASSERT_NOT_NULL(n);
	SP_ASSERT_TRUE(
	n->data == 13 && 
	n->minor->data == 8 && 
	n->major->data == 17 && 
	n->minor->minor->data == 6 && 
	n->minor->major->data == 11
	);
	
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
    nodeFree(n);
}

void test_rbt_delete_3()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	
	nodeDelete_RBT(&n, 17);
	SP_ASSERT_TRUE_WITH_ACTION(
	n->data == 13 &&
	n->major->data == 22 &&
	n->minor->data == 8 &&
	n->major->major->data == 25 &&
	n->major->minor->data == 15, nodeFree(n)
	);
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
    nodeFree(n);
}

void test_bst_delete_3()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	for(int i = 1; i < sizeof(array_ex_1) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	
	Node* r;
	auto xw = nodeDelete_BST(&n, 17, &r);
	
	SP_ASSERT_NOT_NULL_WITH_ACTION(n, nodeFree(n));
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
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
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
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
    nodeFree(n);
}

void test_rbt_delete_6()
{
	Node* n = nodeAlloc(array_ex_3[0]);
	for(int i = 1; i < sizeof(array_ex_3) / sizeof(int); i++)
	{
		nodeAdd_RBT(n, &n, array_ex_3[i]);
	}
	nodeDelete_RBT(&n, 2);
	
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
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
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
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
    nodeFree(n);
}

void test_bst_delete_8()
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
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
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
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
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
	Node* r;
	auto xw = nodeDelete_BST(&n, 27, &r);
	Node* x = xw.first;
	Node* w = xw.second;
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
	nodeDelete_RBT(&n, 27);
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
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
	for(int i = length - 1; i >= 1; i--)
	{
		nodeDelete_RBT(&n, 219);
		verifyRBT(n);
	}
	
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
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

void test_rbt_delete_big_test_1()
{
	Node* n = nodeAlloc(array_big_test_1[0]);
	const SPsize length = sizeof(array_big_test_1) / sizeof(int);
	for(int i = 1; i < length; i++)
	{
		nodeAdd_RBT(n, &n, array_big_test_1[i]);
	}
	
	for(int i = length - 1; i >= 1; i--)
	{
		nodeDelete_RBT(&n, array_big_test_1[i]);
	}
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
	nodePrint(n);
    nodeFree(n);
}

void test_rbt_delete_small_test_1()
{
	Node* n = nodeAlloc(array_ex_1[0]);
	const SPsize length = sizeof(array_ex_1) / sizeof(int);
	for(int i = 1; i < length; i++)
	{
		nodeAdd_RBT(n, &n, array_ex_1[i]);
	}
	nodeDelete_RBT(&n, 13);
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
	nodePrint(n);
    nodeFree(n);
}

//#define PRINT_STEPS
#define ITERATIONS 100
#define LENGTH 100
void test_rbt_delete_random()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dst(1,1000);
    for(int i = 0; i < ITERATIONS; i++)
    {
        Node* n = nodeAlloc(123);

        int* array = new int[LENGTH];
        array[0] = 123;
        SPsize length = LENGTH;
#ifdef PRINT_STEPS
			printf("123 ");
#endif
        for(int i = 1; i < length; i++)
        {

            int num = dst(rng);
            while(containsNumber(num, array, LENGTH))
                num = dst(rng);
#ifdef PRINT_STEPS
			printf("%d ", num);
#endif
            array[i] = num;
            nodeAdd_RBT(n, &n, array[i]);
        }
		
#ifdef PRINT_STEPS
		printf("\n");
#endif
        SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), {nodePrint(n);});
#ifdef PRINT_STEPS
		printf("start:\n");
		nodePrint(n);
#endif
        for(int i = 0; i < LENGTH / 2 - 1; i++)
        {
#ifdef PRINT_STEPS
			printf("deleting %d\n", array[i]);
#endif
            bool status = nodeDelete_RBT(&n, array[i]);
            SP_ASSERT_TRUE_WITH_ACTION(status,
            {
                SP_DEBUG("deletion failed for %d", array[i]);
                delete [] array;
                nodeFree(n);
            });
            SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n),
            {
                SP_DEBUG("imbalanced tree for %d", array[i]);
                printf("data set:\n");
                for(int i = 0; i < LENGTH; i++)
                {
                    printf("%d ", array[i]);
                }
                printf("\n");
                nodePrint(n);
                delete [] array;
                nodeFree(n);
            });
#ifdef PRINT_STEPS
			nodePrint(n);
			printf("\n");
#endif
        }
        for(int i = LENGTH - 1; i >= LENGTH / 2; i--)
        {
#ifdef PRINT_STEPS
			printf("deleting %d\n", array[i]);
#endif
            bool status = nodeDelete_RBT(&n, array[i]);
            SP_ASSERT_TRUE_WITH_ACTION(status,
            {
                SP_DEBUG("deletion failed for %d", array[i]);
                delete [] array;
                nodeFree(n);
            });
            SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n),
            {
                SP_DEBUG("imbalanced tree for %d", array[i]);
                printf("data set:\n");
                for(int i = 0; i < LENGTH; i++)
                {
                    printf("%d ", array[i]);
                }
                printf("\n");
                nodePrint(n);
                delete [] array;
                nodeFree(n);
            });
#ifdef PRINT_STEPS
			nodePrint(n);
			printf("\n");
#endif
        }
        SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
#ifdef PRINT_STEPS
			printf("end:\n");
			nodePrint(n);
			printf("\n");
#endif
        nodeFree(n);
        delete[] array;
	}
	SP_DEBUG("DONE: %d iteration(s) with %d insertion(s)/deletion(s)", ITERATIONS, LENGTH);
}

void test_bst_delete_random()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dst(1,1000);
    for(int i = 0; i < 100; i++)
    {
        Node* n = nodeAlloc(123);

        int* array = new int[LENGTH];
        array[0] = 123;
        SPsize length = LENGTH;
        for(int i = 1; i < length; i++)
        {
            int num = dst(rng);
            while(containsNumber(num, array, LENGTH))
                num = dst(rng);
            array[i] = num;
            nodeAdd_RBT(n, &n, array[i]);
        }
        /*
        printf("\n");
        printf("before:\n");
        nodePrint(n);
        */
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
                        printf("data set:\n");
                        for(int i = 0; i < LENGTH; i++)
                        {
                            printf("%d ", array[i]);
                        }
                        printf("\n");
                        nodeFree(n);
                        delete [] array;
                    }
                );
            }
        }
        nodeFree(n);
        delete[] array;
	}
}

void test_rbt_validity()
{
	Node head;
	Node a;
	a.major = a.minor = NULL;
	Node b;
	b.major = b.minor = NULL;
	Node c;
	c.major = c.minor = NULL;
	head.minor = &a;
	head.major = &b;
	
	
	SP_ASSERT_FALSE(verifyRBT(&head));
}

void test_rbt_delete_small_verification()
{
	int array[] = {123, 825, 835, 67, 206, 832, 271, 505, 547, 972, 472, 317, 209, 87, 800, 223, 772, 216, 756, 688};
	
	Node* n = nodeAlloc(array[0]);
	const SPsize length = sizeof(array) / sizeof(int);
	for(int i = 1; i < length; i++)
	{
		nodeAdd_RBT(n, &n, array[i]);
	}
	
	//nodeDelete_RBT(&n, 261);
	//nodePrint(n);
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
	
	for(int i = length - 1; i >= 1; i--)
	{
		nodeDelete_RBT(&n, array[i]);
		SP_DEBUG("deleting %d", array[i]);
		nodePrint(n);
		SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
	}
	
	nodePrint(n);
    nodeFree(n);
}

void test_rotation_left()
{
	Node* n = nodeAlloc(array_ex_rotation[0]);
	const SPsize length = sizeof(array_ex_rotation) / sizeof(int);
	for(int i = 1; i < length; i++)
	{
		nodeAdd_RBT(n, &n, array_ex_rotation[i]);
	}
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
	n = rotateLeft(n, &n);
	n = rotateLeft(n, &n);
	n = rotateLeft(n, &n);
	n = rotateLeft(n, &n);
	n = rotateLeft(n, &n);
	SP_ASSERT_TRUE_WITH_ACTION(n->data == 493, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(n->minor->data == 352, nodeFree(n));
    nodeFree(n);
}

void test_rotation_right()
{
	Node* n = nodeAlloc(array_ex_rotation[0]);
	const SPsize length = sizeof(array_ex_rotation) / sizeof(int);
	for(int i = 1; i < length; i++)
	{
		nodeAdd_RBT(n, &n, array_ex_rotation[i]);
	}
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
	n = rotateRight(n, &n);
	n = rotateRight(n, &n);
	n = rotateRight(n, &n);
	n = rotateRight(n, &n);
	n = rotateRight(n, &n);
	SP_ASSERT_TRUE_WITH_ACTION(n->data == 112, nodeFree(n));
	SP_ASSERT_TRUE_WITH_ACTION(n->major->data == 136, nodeFree(n));
    nodeFree(n);
}

void test_delete_arbitrary()
{
    int array[] = {123, 242, 866, 925, 443, 365, 787, 662, 773, 651};
	Node* n = nodeAlloc(array[0]);
	const SPsize length = sizeof(array) / sizeof(int);
	for(int i = 1; i < length; i++)
	{
		nodeAdd_RBT(n, &n, array[i]);
	}
    nodePrint(n);
    SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n),
	{
            SP_WARNING("imbalance at build");

            nodeFree(n);
	});

	SP_DEBUG("deletion:");
    for(int i = 0; i < length; i++)
	{
	    SP_DEBUG("deleting %d", array[i]);
		SP_ASSERT_TRUE_WITH_ACTION(nodeDelete_RBT(&n, array[i]),
		{
		    SP_WARNING("error while deleting %d", array[i]);
		    nodeFree(n);
		});
		nodePrint(n);
		SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n),
		{
            SP_WARNING("rbt-violation after deleting %d", array[i]);
            nodeFree(n);
		});
	}
	SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), nodeFree(n));
    nodeFree(n);
}

void test_verification_random()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dst(1,1000);

	Node* n = nodeAlloc(123);
	
	int* array = new int[LENGTH];
	array[0] = 123;
	SPsize length = LENGTH;
	SP_DEBUG("adding %d", array[0]);
	for(int i = 1; i < length; i++)
	{
	    int num = dst(rng);
	    while(containsNumber(num, array, LENGTH))
	        num = dst(rng);
		array[i] = num;
		SP_DEBUG("adding %d", array[i]);
		nodeAdd_RBT(n, &n, array[i]);
		SP_ASSERT_TRUE_WITH_ACTION(verifyRBT(n), 
			{
				nodePrint(n);
				nodeFree(n); 
				delete [] array;
			}
		);
	}
	
    nodeFree(n);
	delete[] array;
}

void test_breadth_first_traversal()
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
	nodePrint(n);
	
	std::queue<const Node*> nodes = iterate(n);
	SP_DEBUG("iterate done");
	while(!nodes.empty())
	{
		const Node* n = nodes.front();
		printf("n: %d\n", n->data);
		nodes.pop();
	}
    nodeFree(n);
}

int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
    //SP_TEST_ADD(test_find);
	
	// fail 9, 10
    /*
	SP_TEST_ADD(test_rbt_delete_1);
	
	SP_TEST_ADD(test_bst_delete_9);
    SP_TEST_ADD(test_rbt_delete_9);
    
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
    */
    //SP_TEST_ADD(test_bst_delete_random);
    //SP_TEST_ADD(test_delete_arbitrary);

	//SP_TEST_ADD(test_delete_arbitrary);
	//SP_TEST_ADD(test_verification_random);
	//SP_TEST_ADD(test_rotation_left);
	
	//SP_TEST_ADD(test_rbt_delete_small_verification);
	//SP_TEST_ADD(test_rbt_delete_random);
	SP_TEST_ADD(test_breadth_first_traversal);
    /*
    SP_TEST_ADD(test_bst_delete_no_children_no_parent);
    SP_TEST_ADD(test_bst_delete_one_child_no_parent);
    SP_TEST_ADD(test_bst_delete_one_child_and_parent);
    SP_TEST_ADD(test_bst_delete_two_children_major_minimum);
    SP_TEST_ADD(test_bst_delete_two_children_major_minor_minimum);

	
    SP_TEST_ADD(test_rbt_delete_big_test_1);
    SP_TEST_ADD(test_bst_delete_two_children_x_minimum);
    */
    //SP_TEST_ADD(test_bst_delete_1);
    //SP_TEST_ADD(test_rbt_delete_small_test_1);
	
	//SP_TEST_ADD(test_rbt_validity);
	
	spTestRunAll();
	spTestTerminate();
	return 0;
}