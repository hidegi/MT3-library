#include <SP/test/unit.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

unsigned long long values[][20] =
{
	{554, 865, 140, 9, 893, 859, 616, 146, 559, 166, 	852, 525, 538, 241, 540, 994, 699, 730, 701, 548}
};
struct Node
{
    bool red;
    unsigned long long data;
    Node* parent;
    Node* major;
    Node* minor;
};
	
    
bool isMajor(Node* n)
{
    return (n && n->parent) ? (n->parent->major == n) : SP_FALSE;
}

bool isMinor(Node* n)
{
    return (n && n->parent) ? (n->parent->minor == n) : SP_FALSE;
}

bool isRoot(Node* n)
{
    return (n && !n->parent);
}

Node* nodeFindMember(Node* n)
{
    if(isRoot(n->parent))
    {
	printf("impossible state??");
	return NULL;
    }

    return (isMajor(n->parent)) ? n->parent->parent->minor : n->parent->parent->major;
}

Node* nodeAlloc(unsigned long long value)
{
    Node* n = new Node;
    n->data = value;
    n->major = n->minor = n->parent = NULL;
    n->red = false;
    return n;
}

void nodeFree(Node* n)
{
    if(n)
    {
	//data free impl..
	nodeFree(n->major);
	nodeFree(n->minor);
	delete n;
    }
}

void nodePrintImpl(Node* n, int level)
{
    if(n)
    {
	for(int i = 0; i < level; i++)
	    printf("\t");
	printf("(%c) (%c) %lld\n", n->red ? 'R' : 'B', isRoot(n) ? '*' : isMajor(n) ? '+' : '-', n->data);
	nodePrintImpl(n->major, level + 1);
	nodePrintImpl(n->minor, level + 1);
    }
}

void nodePrint(Node* n)
{
    nodePrintImpl(n, 0);
}

/*
 *	deletion rule:
 *
 *	- if to be deleted node has no children, replacement is NULL..
 *	- if to be deleted node has one child, replacement is child..
 *	- if to be deleted node has two children:
 *	
 */
 


Node* nodeFind(Node* head, unsigned long long value)
{	
	if(head)
	{
		if(value == head->data)
			return head;
		
		if(value > head->data)
		{
			return nodeFind(head->major, value);
		}
		else
		{
			return nodeFind(head->minor, value);
		}
	}
	return NULL;
}

void nodeDeleteImpl_RBT(Node* n)
{
	if(n)
	{
		bool maj = isMajor(n);
		if(!n->major && !n->minor)
		{
			if(maj)
			{
				n->parent->major = NULL;
			}
			else
			{
				n->parent->minor = NULL;
			}
			delete n;
		}
		else if((n->major && !n->minor) || (!n->major && n->minor))
		{
			Node** m = maj ? &n->parent->major : &n->parent->minor;
			if(n->major)
			{
				*m = n->major;
			}
			else
			{
				*m = n->minor;
			}
			delete n;
		}
	}
}

Node* findMajorMost(Node* n)
{
	if(n)
	{
		if(!n->major)
			return n;
		else
			return findMajorMost(n->major);
	}
	return NULL;
}

Node* findMinorMost(Node* n)
{
	if(n)
	{
		if(!n->minor)
			return n;
		else
			return findMinorMost(n->minor);
	}
	return NULL;
}

Node* nodeDeleteImpl_BST(Node* n);
void nodeAdd_RBT(Node* node, Node** head, unsigned long long value);
void nodeDelete_BST(Node** head, unsigned long long value);
void fixViolations(Node* node, Node** head);
void nodeDelete_RBT(Node** head, unsigned long long value);

bool containsNumber(int n, const int* array, int length)
{
    for(int i = 0; i < length; i++)
        if(array[i] == n)
            return true;
    return false;
}
#define LENGTH 100
void test_delete()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dst(1,1000);

	Node* n = nodeAlloc(123);
	
	/*int array[] = {123, 539, 503, 941, 262, 559, 839, 352, 461, 620, 517, 128, 734, 466, 591, 353, 241, 317, 316, 709, 320, 346, 814, 190, 5, 6, 639, 230, 428, 872, 195, 768, 686, 497, 173, 237, 866, 104, 424, 815, 521, 254, 124, 819, 489, 339, 374, 949, 386, 509, 347, 222, 552, 280,
	                    238, 850, 161, 942, 289, 905, 516, 69, 57, 493, 207, 275, 740, 462, 995, 88, 875, 728, 233, 120, 719, 749, 901, 544, 366, 74, 967, 787, 970, 714, 422, 946, 582, 483, 832, 144, 148, 25, 919, 663, 578};
	*/

	int* array = new int[LENGTH];
	array[0] = 123;
	SPsize length = LENGTH; //sizeof(array) / sizeof(int);
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

	for(int i = length - 1; i >= 1; i--)
	{
		nodeDelete_RBT(&n, array[i]);
	}

	//nodeDelete_BST(&n, array[1]);
	//nodeDelete_BST(&n, array[2]);
	printf("\n\nafter:\n");
	nodePrint(n);
    nodeFree(n);
	delete[] array;
}

Node* nodeDeleteImpl_BST(Node* n, Node** head)
{
    Node* m = NULL;
	if(n)
	{	
		bool maj = isMajor(n);
		bool rt  = isRoot(n);

		if(!n->major && !n->minor)
		{
		    // case 1: no children..
			if(n->parent)
				maj ? (n->parent->major = NULL) : (n->parent->minor = NULL);
		}
		else if((n->major && !n->minor) || (!n->major && n->minor))
		{
		    // case 2: one child..
            m = n->major ? n->major : n->minor;
		    Node* mP = n->parent;

            m->parent = !rt ? n->parent : NULL;
            if(!rt)
                (maj) ? (mP->major = m) : (mP->minor = m);
		}
		else
		{
		    // case 3: two children..
            m = findMinorMost(n->major);
		    Node* mP = m->parent;
            Node* mMj = m->major;

            Node* nP  = n->parent;
            Node* nMj = n->major;
            Node* nMn = n->minor;

		    if(!rt && nP)
		        maj ? (nP->major = m) : (nP->minor = m);

            m->parent = !rt ? nP : NULL;
            m->minor = nMn;
            nMn->parent = m;

            if(m != nMj)
            {
                mP->minor = mMj;
                nMj->parent = m;
                if(mMj)
                   mMj->parent = mP;
                m->major = nMj;
            }
		}

		if(rt)
		    *head = m;

		delete n;
	}
	return m;
}

void nodeDelete_RBT(Node** head, unsigned long long value)
{
	Node* n = nodeFind(*head, value);
	if(n)
	{
        bool nRed = n->red;

        Node* r = nodeDeleteImpl_BST(n, head);

        if(r)
        {
            bool rRed = r->red;
            if(!(nRed && rRed))
            {
                if(!nRed != !rRed)
                    rRed = !rRed;
                //continue here..
            }
        }
	}
}

void nodeDelete_BST(Node** head, unsigned long long value)
{
	Node* n = nodeFind(*head, value);
	nodeDeleteImpl_BST(n, head);
}
void nodeAdd_BST(Node* node, unsigned long long value)
{
    if(node)
    {
	if(node->data == value)
	{
	    printf("Node %d already available\n", value);
	    return;
	}

	if(value > node->data)
	{
	    if(node->major)
	    {
		nodeAdd_BST(node->major, value);
	    }
	    else
	    {
		node->major = nodeAlloc(value);
		node->major->parent = node;
		node->red = true;
	    }
	}
	else
	{
	    if(node->minor)
	    {
		nodeAdd_BST(node->minor, value);
	    }
	    else
	    {
		node->minor = nodeAlloc(value);
		node->minor->parent = node;
		node->red = true;
	    }
	}
    }
}

Node* rotateLeft(Node* n, Node** head)
{
    bool maj = isMajor(n);
    Node* parent = n->parent;
    Node* major = n->major;
    Node* carry = n->major->minor;

    n->parent = major;
    n->major->minor = n;
    if(carry)
       carry->parent = n;
    n->major = carry;

    major->parent = parent;

    if(parent)
    {
	if(maj)
	    parent->major = major;  //n was a subtree..
	else
	    parent->minor = major;
    }
    else
    {
	*head = major; //n was the root node..
    }

    return major;
}

Node* rotateRight(Node* n, Node** head)
{
    bool maj = isMajor(n);
    Node* parent = n->parent;
    Node* minor = n->minor;
    Node* carry = n->minor->major;

    n->parent = minor;
    n->minor->major = n;
    if(carry)
       carry->parent = n;
    n->minor = carry;

    minor->parent = parent;

    if(parent)
    {
	if(maj)
	    parent->major = minor;  //n was a subtree..
	else
	    parent->minor = minor;
    }
    else
    {
	*head = minor; //n was the root node..
    }

    return minor;
}

void fixViolations(Node* node, Node** head)
{
    if(node)
    {
		if(node->red && node->parent->red)
		{
			Node* member = nodeFindMember(node);
			Node* parent = node->parent;
			Node* grandParent = parent->parent;

			bool isRed = member != NULL ? member->red : false;
			if(isRed)
			{
				//color switch parent, parent-sibling and grandparent..
				parent->red = !parent->red;
				grandParent->red = grandParent->parent ? !grandParent->red : false;
				member->red = !member->red;
				fixViolations(grandParent, head);
			}
			else
			{
				/*
				 *  child-parent combinations:
				 *
				 *  0 = minor..
				 *  1 = major..
				 *
				 *  P   C | rotation
				 * -------|----------
				 *  0   0 |    RR
				 *  0   1 |    LR
				 *  1   0 |    RL
				 *  1   1 |    LL
				 *
				 */
				// RR rotation..
				if(isMinor(parent) && isMinor(node))
				{
					//printf("RR %d\n", node->data);
					Node* next = rotateRight(node->parent->parent, head);

					next->red = !next->red;
					next->major->red = !next->major->red;
					fixViolations(next, head);
					return;
				}

				// LR-rotation..
				if(isMinor(parent) && isMajor(node))
				{
					//printf("LR %d\n", node->data);
					//new node becomes root..
					//major is grand-parent..
					//minor is parent..
					Node* next = rotateLeft(node->parent, head);
								 rotateRight(next->parent, head);
					next->red = !next->red;
					next->major->red = !node->major->red;
					fixViolations(next, head);
					return;
				}

				// RL-rotation..
				if(isMajor(parent) && isMinor(node))
				{
					//printf("RL %d\n", node->data);
					/*
					printf("grandparent: %d, parent: %d, child: %d\n", node->parent->parent->data, node->parent->data, node->data);
					printf("parent minor: %lld\n", parent->minor->data);
					*/

					Node* next = rotateRight(node->parent, head);
								 rotateLeft(next->parent, head);
					next->red = !next->red;
					next->minor->red = !node->minor->red;

					//printf("RL ok\n");
					fixViolations(next, head);
					return;
				}

				// LL-rotation
				if(isMajor(parent) && isMajor(node))
				{
					//printf("LL %d\n", node->data);
					Node* next = rotateLeft(node->parent->parent, head);
					next->red = !next->red;
					next->minor->red = !next->minor->red;

					fixViolations(next, head);
					return;
				}
			}
		}
    }
}

void nodeAdd_RBT(Node* node, Node** head, unsigned long long value)
{
    if(node)
    {
	if(node->data == value)
	{
	    printf("Node %d already available\n", value);
	    return;
	}

	if(value > node->data)
	{
	    if(node->major)
	    {
		nodeAdd_RBT(node->major, head, value);
	    }
	    else
	    {
		node->major = nodeAlloc(value);
		node->major->parent = node;
		node->major->red = true;
		fixViolations(node->major, head);
	    }
	}
	else
	{
	    if(node->minor)
	    {
		nodeAdd_RBT(node->minor, head, value);
	    }
	    else
	    {
		node->minor = nodeAlloc(value);
		node->minor->parent = node;
		node->minor->red = true;
		fixViolations(node->minor, head);
	    }
	}
    }
}

/*
 *  whenever there is triangle, rotate right-left or left-right (2 rotations)..
 *  whenever there is skew, rotate left or right (1 rotation)..
 *  rotate always around the parent of the newly added node..
 */
void roatateLeftImpl()
{
}


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

int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
    //SP_TEST_ADD(test_find);
    SP_TEST_ADD(test_delete);

	spTestRunAll();
	spTestTerminate();
	return 0;
}