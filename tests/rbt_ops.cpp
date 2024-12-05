#include <SP/config.h>
#include <utility>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "rbt.h"
#define MT3_HAVE_BST_MAJOR_INCLINED

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wformat"
#endif

Node* rotateLeft(Node* n, Node** head);
Node* rotateRight(Node* n, Node** head);
void fixViolations(Node* node, Node** head);
bool procedure_2(Node* x, Node* w, Node** head);
bool procedure_3(Node* x, Node* w, Node** head);
bool procedure_4(Node* x, Node* w, Node** head);
bool nodeFixupRules_RBT(Node* x, Node* w, Node** head);

/*
 *	internal test API
 */ 
bool isMajor(Node* n)
{
    return (n && n->parent) ? (n->parent->major == n) : SP_FALSE;
}

bool isMinor(Node* n)
{
    return (n && n->parent) ? (n->parent->minor == n) : SP_FALSE;
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

Node* getSibling(Node* n)
{
	if(!n || !n->parent)
		return NULL;
	
	bool maj = isMajor(n);
	return maj ? n->parent->minor : n->parent->major;
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

void nodePrintImpl(Node* n, int level)
{
    if(n)
    {
		if(n->major)
			SP_ASSERT(n == n->major->parent, "Illegal major linking for %d", n->data);
		if(n->minor)
			SP_ASSERT(n == n->minor->parent, "Illegal minor linking for %d", n->data);
		
		
		for(int i = 0; i < level; i++)
			printf("\t");
		printf("(%c) (%c) %lld\n", n->red ? 'R' : 'B', isRoot(n) ? '*' : isMajor(n) ? '+' : '-', n->data);
		nodePrintImpl(n->major, level + 1);
		nodePrintImpl(n->minor, level + 1);
    }
}

// BST delete implementation..
std::pair<Node*, Node*> nodeDeleteImpl_BST(Node* n, Node** head, Node** replacement)
{
    Node* x = NULL;
	Node* w = NULL;
	if(n)
	{	
		Node* r = NULL;
		Node* p = n->parent;
		
		bool maj = isMajor(n);
		bool rt  = isRoot(n);

		if(!n->major && !n->minor)
		{
		    // case 1: no children..
			// x is NULL, set sibling, if available..
			if(p)
			{
				maj ? (p->major = NULL) : (p->minor = NULL);
				w = maj ? (p->minor) : (p->major);
			}
			else
			{
				SP_ASSERT(rt, "Expected deleted node to be root");
			}
		}
		else if((n->major && !n->minor) || (!n->major && n->minor))
		{
		    // case 2: one child..
            r = n->major ? n->major : n->minor;
			SP_ASSERT(r, "Expected to have replacement");
			if(r)
               r->parent = p;

            if(r)
			{
				if(maj)
				{
				    if(p)
					p->major = r;
					x = r->major;
					w = r->minor;
				}
				else
				{
                    if(p)
					p->minor = r;
					x = r->minor;
					w = r->major;
				}
			}
			*replacement = r;
		}
		else
		{
			// case 3: two children..
#ifndef MT3_HAVE_BST_MAJOR_INCLINED
			r = findMinorMost(n->major);
			w = r->parent ? (isMajor(r) ? r->parent->minor : r->parent->major) : NULL;
			
			SP_ASSERT(r, "Expected to have replacement");
			SP_ASSERT(!r->minor, "Expected to have minor-most replacement (x)");
			
			Node* q = n->minor;
			Node* m = n->major;
			
			Node* a = r->parent;
			Node* b = r->major;
			
			SP_ASSERT(!r->minor, "Expected to have no more minimums");
			
			// link p and r..
			if(p)
				maj ? (p->major = r) : (p->minor = r);
			r->parent = p;
			
			// link q and r..
			if(q)
			   q->parent = r;
			r->minor = q;
			
			if(r != m)
			{
				SP_ASSERT(a, "Expected minimum to have parent");
				// link m and r..
				r->major = m;
				m->parent = r;
				
				// link a and b..
				a->minor = b;
				if(b)
				   b->parent = a;
			}
#else
			r = findMajorMost(n->minor);
			w = r->parent ? (isMajor(r) ? r->parent->minor : r->parent->major) : NULL;
			
			SP_ASSERT(r, "Expected to have replacement");
			SP_ASSERT(!r->major, "Expected to have minor-most replacement (x)");
			
			Node* q = n->major;
			Node* m = n->minor;
			
			Node* a = r->parent;
			Node* b = r->minor;
			
			SP_ASSERT(!r->major, "Expected to have no more minimums");
			
			// link p and r..
			if(p)
				maj ? (p->major = r) : (p->minor = r);
			r->parent = p;
			
			// link q and r..
			if(q)
			   q->parent = r;
			r->major = q;
			
			if(r != m)
			{
				SP_ASSERT(a, "Expected minimum to have parent");
				// link m and r..
				r->minor = m;
				m->parent = r;
				
				// link a and b..
				a->major = b;
				if(b)
				   b->parent = a;
			}
#endif
			x = b;
		}
		
		*replacement = r;
		if(!p)
			*head = r;
		delete n;
	}

	return std::make_pair(x, w);
}

// procedure steps 0 to 4..
bool procedure_0(Node* x)
{
	if(x->red)
	{
		//SP_DEBUG("proc 0");
		x->red = false;
		return true;
	}
	return false;
}

bool procedure_1(Node* x, Node* w, Node** head)
{
    if(w)
    {
        bool xBlack = x ? !x->red : true;
        // only x could be double black, w must be red..
        bool maj = !isMajor(w);
        if(xBlack && w->red)
        {
            // w cannot be double black..
            SP_ASSERT(w->parent, "Fatal, replacement expected to have parent");
            SP_ASSERT(maj ? w->parent->minor == w : w->parent->major == w, "Linking error");
            if(x)
            {
                SP_ASSERT(w->parent == x->parent, "Fatal, replacement and sibling expected to have equal parent");
            }
            /*
             *	---- 1 ----
             *	1. colour w black..
             *	2. colour x's parent red..
             *	3. rotate x's parent:
             *		if x is major, then rotate right..
             *		if x is minor, then rotate left..
             *	4. set w to x's sibling..
             *	5. decide case 2, 3 or 4 from this state..
             */

            // 1. colour w black..
            w->red = false;

            // 2. colour x's parent red..
            w->parent->red = true;
            /*
             *	3. rotate x's parent:
             *		if x is major, then rotate right..
             *		if x is minor, then rotate left..
             */
            Node* m = maj ? w->major : w->minor;
            Node* p = maj ? rotateRight(w->parent, head) : rotateLeft(w->parent, head);

            SP_ASSERT(p, "Expeceted to have rotation replacement");
            SP_ASSERT(p == w, "Rotation error");
            p = maj ? p->major : p->minor;

            // 4. set w to x's sibling..
            SP_ASSERT(p, "Expected to have parent");
            x = maj ? p->major : p->minor;
            w = maj ? p->minor : p->major;
            SP_ASSERT(m == w, "Rotation error");
            // 5. decide case 2, 3 or 4 from this state..

            /*
            if(procedure_2(x, w, head))
                return true;

            if(procedure_3(x, w, head))
                return true;
            SP_DEBUG("proc 1 to proc 4");
            return procedure_4(x, w, head);
            */
            return nodeFixupRules_RBT(x, w, head);
        }
    }
	return false;
}

bool procedure_2(Node* x, Node* w, Node** head)
{	
	// x is black and w is black..
	// x could be double black, w cannot be double black..
	if(w)
	{
        bool xBlack = x ? !x->red : true;
        if(xBlack && !w->red)
        {
            bool maj = !isMajor(w);
            bool wMnB = w->minor ? !w->minor->red : true;
            bool wMjB = w->major ? !w->major->red : true;

            if(wMnB && wMjB)
            {
                // only x could be double black..
                /*
                 *	1. colour w red..
                 *	2. set x to its parent..
                 *		if x is red, colour x black (return)..
                 *		if x is black and root, return..
                 *		if x is black and not root, decide on case 1, 2, 3, 4..
                 */

                // 1. colour w red..
                w->red = true;

                // set x to its parent..
                x = w->parent;
                if(isRoot(x))
                {
                    x->red = false;
                    return true;
                }

                maj = isMajor(x);
                w = maj ? x->parent->minor : x->parent->major;

                SP_ASSERT(x, "Replacement expected to have parent");

                if(x->red)
                {
                    // x is red, colour x black (return)..
                    x->red = false;
                    return true;
                }
                else
                {
                    // x is black..
                    // x is not root, decide on case 1, 2, 3, 4..
                    w = maj ? w->parent->minor : w->parent->major;
                    x = maj ? w->parent->major : w->parent->minor;

                    /*
                    if(procedure_1(x, w, head))
                        return true;
                    if(procedure_2(x, w, head))
                        return true;
                    if(procedure_3(x, w, head))
                        return true;
                    return procedure_4(x, w, head);
                    */
                    return nodeFixupRules_RBT(x, w, head);
                }
            }
        }
    }

	return false;
}

bool procedure_3(Node* x, Node* w, Node** head)
{
    if(w)
    {
        // x could be double black, w cannot be double black..
        bool xBlack = x ? !x->red : true;
        bool wMjB = w->major ? !w->major->red : true;
        bool wMnB = w->minor ? !w->minor->red : true;

        bool maj = !isMajor(w);
        if(xBlack && (maj ? (!wMjB && wMnB) : (wMjB && !wMnB)))
        {
            if(x)
            {
                SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
            }
            /*
             *	1. colour w's child black:
             *		if x is major, then colour w's major black..
             *		if x is minor, then colour w's minor black..
             *	2. colour w red..
             *	3. rotate w:
             *		if x is minor, rotate right..
             *		if x is major, rotate left..
             *	4. set w to x's sibling..
             *	5. proceed to case 4..
             */
            maj = !isMajor(w);

            /*
             *	1. colour w's child black:
             *		if x is major, then colour w's major black..
             *		if x is minor, then colour w's minor black..
             */
            SP_ASSERT(maj ? w->major : w->minor, "Expected sibling's child");
            maj ? (w->major->red = false) : (w->minor->red = false);

            // 2. colour w red..
            w->red = true;

            /*
             *	3. rotate w:
             *		if x is major, rotate left..
             *		if x is minor, rotate right..
             */
            w = maj ? rotateLeft(w, head) : rotateRight(w, head);
            SP_ASSERT(w, "Expected sibling");
            SP_ASSERT(w->parent, "Expected parent");

            // x could be double black..
            x = maj ? w->parent->major : w->parent->minor;

            // 4. set w to x's sibling..
            SP_ASSERT(w != x, "Sibling and replacement cannot be the same");
            // 5. proceed to case 4..
            return procedure_4(x, w, head);
        }
	}
	return false;
}

bool procedure_4(Node* x, Node* w, Node** head)
{
    if(w)
    {
        // (w is black) and (isMajor(x) ? w's major is red : w's minor is red) (soft check)..
        bool wMjR = w->major ? w->major->red : false;
        bool wMnR = w->minor ? w->minor->red : false;
        bool xBlack = x ? !x->red : true;
        bool maj = !isMajor(w);
        if(xBlack && (maj ? wMnR : wMjR))
        {
            if(x)
            {
                SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
            }
            /*
             *	1. w's colour is x's parent's colour..
             *	2. x's parent is black..
             *	3. colour w's child black:
             *		if x is major, w's minor is black..
             *		if x is minor, w's major is black..
             *	4. rotate x's parent:
             *		if x is major, rotate right..
             *		if x is minor, rotate left..
             */
            SP_ASSERT(w->parent, "Expected to have parent");
            maj = !isMajor(w);
            // 1. w's colour is x's parent's colour..
            w->red = w->parent->red;

            // 2. x's parent is black..
            w->parent->red = false;

            /*	3. colour w's child black:
             *		if x is major, w's minor is black..
             *		if x is minor, w's major is black..
             */
            SP_ASSERT(maj ? w->minor : w->major, "Expected sibling child");
            maj ? (w->minor->red = false) : (w->major->red = false);

            /*
             *	4. rotate x's parent:
             *		if x is major, rotate right..
             *		if x is minor, rotate left..
             */
            maj ? rotateRight(w->parent, head) : rotateLeft(w->parent, head);
            return true;
        }
	}
	return false;
}

// fix up rules..
bool nodeFixupRules_RBT(Node* x, Node* w, Node** head)
{
    bool xRed = x ? x->red : false;
	if(xRed)
	{
	    // x is red -> case 0
		return procedure_0(x);
	}
	else
	{
	    if(!w)
	    {
	        // following cases would expect w to have children..
	        // since double black cannot have children, return here..
	        return true;
        }
        bool maj = !isMajor(w);
	    // x could be double black..
		if(w->red)
		{
		    // x is black and w is red..
			return procedure_1(x, w, head);
		}
		else
		{

			bool wMjB = w->major ? !w->major->red : true;
			bool wMnB = w->minor ? !w->minor->red : true;
			if(wMnB && wMjB)
			{
			    // x is black and w has two black children..
				return procedure_2(x, w, head);
			}


			if(maj ? (!wMjB && wMnB) : (!wMnB && wMjB))
			{
			    // x is black and w's farthest child to x is black and w's nearest child to x is red..
				return procedure_3(x, w, head);
			}

			if(maj ? !wMnB : !wMjB)
			{
				return procedure_4(x, w, head);
			}

			SP_ASSERT(0, "Expected procedure");
		}
	}
	
	return true;
}


// fix up function..
// colouring..

/*
 *  steps:
 *  1. if replacement is NULL, then return, since it was the root without replacement..
 *  2. if x is double black and the replacement is root, color the replacement black..
 *
 */
bool nodeFixup_RBT(bool rBefore, Node* r, Node* x, Node* w, Node** head)
{
    // caveats: x and w could be double black..
	bool rAfter = r ? r->red : false;
	
	// 1. rule: deleted node is red, and replcament is red or NULL => return..
	if(rBefore && rAfter)
	{
		// R -> R
		return true;
	}
	
	// 2. rule: deleted node is black and replacement is red
	// => colour replacement black and return..
	if(!rBefore && rAfter)
	{
		// B -> R
		if(r)
		   r->red = false;
		return true;
	}
	
	
	// 3. rule: deleted node is red and replacement is black and not NULL 
	// => colour replacement red and proceed to appropriate case..
	if(rBefore && !rAfter)
	{
		// R -> B
		if(r)
		   r->red = true;
		return nodeFixupRules_RBT(x, w, head);
	}
	

	// 4. rule: deleted node is black and replcaement is black..
	if(!rBefore && !rAfter)
	{
		// if replacement is not root, then proceed to appropriate case..
		// x could be double black..
		if(isRoot(x))
		{
		    return true;
		}

        return nodeFixupRules_RBT(x, w, head);
	}

	return false;
}


/*
 *  whenever there is triangle, rotate right-left or left-right (2 rotations)..
 *  whenever there is skew, rotate left or right (1 rotation)..
 *  rotate always around the parent of the newly added node..
 */
Node* rotateLeft(Node* n, Node** head)
{
	SP_ASSERT(n, "Expected rotation node");
    bool maj = isMajor(n);
    Node* parent = n->parent;
    Node* major = n->major;
    Node* carry = major ? n->major->minor : NULL;

    n->parent = major;
	if(n->major)
		n->major->minor = n;
    if(carry)
       carry->parent = n;
    n->major = carry;
	
	if(major)
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
	SP_ASSERT(n, "Expected rotation node");
    bool maj = isMajor(n);
    Node* parent = n->parent;
    Node* minor = n->minor;
    Node* carry = minor ? n->minor->major : NULL;
	
    n->parent = minor;
	if(n->minor)
		n->minor->major = n;
	
    if(carry)
       carry->parent = n;
    n->minor = carry;
	if(minor)
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
					/*
					printf("grandparent: %d, parent: %d, child: %d\n", node->parent->parent->data, node->parent->data, node->data);
					printf("parent minor: %lld\n", parent->minor->data);
					*/

					Node* next = rotateRight(node->parent, head);
								 rotateLeft(next->parent, head);
					next->red = !next->red;
					next->minor->red = !node->minor->red;

					fixViolations(next, head);
					return;
				}

				// LL-rotation
				if(isMajor(parent) && isMajor(node))
				{
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

/*
 *	use test API
 */ 
bool nodeDelete_RBT(Node** head, unsigned long long value)
{
	Node* n = nodeFind(*head, value);
	if(n)
	{
        bool aRed = n->red;
		Node* replacement = NULL;
        auto xw = nodeDeleteImpl_BST(n, head, &replacement);
		
		
		Node* r = NULL;
		Node* x = xw.first;
		Node* w = xw.second;
		if(x && w)
		{
			SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
		}

		return nodeFixup_RBT(aRed, replacement, x, w, head);
	}
	return true;
}

std::pair<Node*, Node*> nodeDelete_BST(Node** head, unsigned long long value, Node** replacement)
{
	Node* n = nodeFind(*head, value);
	if(n)
	{
		SP_ASSERT(n->data == value, "Ill-defined search algorithm");
		Node* dummy;
		return nodeDeleteImpl_BST(n, head, replacement);
	}
	return std::make_pair<Node*, Node*>(NULL, NULL);
}
Node* nodeAlloc(unsigned long long value)
{
    Node* n = new Node;
    n->data = value;
    n->major = n->minor = n->parent = NULL;
    n->red = false;
    return n;
}

void nodePrint(Node* n)
{
    nodePrintImpl(n, 0);
	printf("\n\n");
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

void collect(const Node* head, std::queue<const Node*>& q)
{
	if(head)
	{
		
		if(head->minor)
			collect(head->minor, q);
		
		if(head->major)
			collect(head->major, q);
		q.push(head);
	}
}

std::queue<const Node*> iterate(const Node* head)
{
	std::queue<const Node*> result;
	if(head)
	{
		collect(head, result);
	}
	return result;
}

#if defined(SP_COMPILER_CLANG) || defined(SP_COMPILER_GNUC)
#pragma GCC diagnostic pop
#endif