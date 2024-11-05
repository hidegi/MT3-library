#include <SP/test/unit.h>
#include <iostream>

struct Node
{
    bool red;
    int data;
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

Node* nodeAlloc(int value)
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
        printf("(%c) (%c) %d\n", n->red ? 'R' : 'B', isRoot(n) ? '*' : isMajor(n) ? '+' : '-', n->data);
        nodePrintImpl(n->major, level + 1);
        nodePrintImpl(n->minor, level + 1);
    }
}

void nodePrint(Node* n)
{
    nodePrintImpl(n, 0);
}

void nodeAdd_BST(Node* node, int value)
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
    printf("rotate left\n");
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
    printf("rotate right\n");
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
    if(!node)
    {
        printf("node = NULL\n");
        return;
    }
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
                printf("RR %d\n", node->data);
                Node* next = rotateRight(node->parent->parent, head);

                next->red = !next->red;
                next->major->red = !next->major->red;
                fixViolations(next, head);
                return;
            }

            // LR-rotation..
            if(isMinor(parent) && isMajor(node))
            {
                printf("LR %d\n", node->data);
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
                printf("RL %d\n", node->data);
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
                printf("LL %d\n", node->data);
                Node* next = rotateLeft(node->parent->parent, head);
                next->red = !next->red;
                next->minor->red = !next->minor->red;

                fixViolations(next, head);
                return;
            }
        }
    }
}

void nodeAdd_RBT(Node* node, Node** head, int value)
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
    Node* n = nodeAlloc(2);

    nodeAdd_RBT(n, &n, 65);
    nodeAdd_RBT(n, &n, 52);
    nodeAdd_RBT(n, &n, 6);

    nodeAdd_RBT(n, &n, 38);

    nodeAdd_RBT(n, &n, 34);

    nodeAdd_RBT(n, &n, 68);


    nodeAdd_RBT(n, &n, 40);

    nodeAdd_RBT(n, &n, 85);

    nodeAdd_RBT(n, &n, 72);

    nodeAdd_RBT(n, &n, 70);

    nodeAdd_RBT(n, &n, 39);

    nodeAdd_RBT(n, &n, 16);

    nodeAdd_RBT(n, &n, 24);

    nodeAdd_RBT(n, &n, 54); //correct


    nodeAdd_RBT(n, &n, 26);

    nodeAdd_RBT(n, &n, 28);

    nodeAdd_RBT(n, &n, 89);

    nodeAdd_RBT(n, &n, 45);
    nodeAdd_RBT(n, &n, 97);

    nodePrint(n);
    nodeFree(n);
}

int main(int argc, char** argv)
{
	SP_TEST_INIT(argc, argv);
    SP_TEST_ADD(test_tree_insert);

	spTestRunAll();
	spTestTerminate();
	return 0;
}