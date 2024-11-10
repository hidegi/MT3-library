#include <SP/config.h>
#include <utility>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "rbt.h"
#define MOT_HAVE_SBT_MAJOR_INCLINED

Node* rotateLeft(Node* n, Node** head);
Node* rotateRight(Node* n, Node** head);
void fixViolations(Node* node, Node** head);
bool procedure_2(Node** x, Node** w, Node** head);
bool procedure_3(Node** x, Node** w, Node** head);
bool procedure_4(Node** x, Node** w, Node** head);

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
		bool maj = isMajor(n);
		bool rt  = isRoot(n);

		if(!n->major && !n->minor)
		{
		    // case 1: no children..
			// x is NULL, set sibling, if available..
			if(n->parent)
			{
				maj ? (n->parent->major = NULL) : (n->parent->minor = NULL);
				w = maj ? (n->parent->minor) : (n->parent->major);
			}
			else
			{
				SP_ASSERT(rt, "Expected deleted node to be root");
				*head = NULL;
			}
			*replacement = NULL;
		}
		else if((n->major && !n->minor) || (!n->major && n->minor))
		{
		    // case 2: one child..
            Node* r = n->major ? n->major : n->minor;
			SP_ASSERT(r, "Expected to have replacement");
			
		    Node* p = n->parent;
            r->parent = p;
			
            if(p)
			{
                (maj) ? (p->major = r) : (p->minor = r);
				if(maj)
				{
					x = p->major;
					w = p->minor;
				}
				else
				{
					x = p->minor;
					w = p->major;
				}
			}
			else
			{
				*head = r;
			}
			
			*replacement = r;
		}
		else
		{
			Node* p = n->parent;
			Node* r = findMinorMost(n->major);
				  w = r->parent ? (isMajor(r) ? r->parent->minor : r->parent->major) : NULL;
			
			SP_ASSERT(r, "Expected to have replacement");
			SP_ASSERT(!r->minor, "Expected to have minor-most replacement (x)");
			
			Node* q = n->minor;
			Node* m = n->major;
			if(r == m)
			{
				// bst delete case 3.1
				SP_ASSERT(!r->minor, "Expected to have no more minimums");
				// link p and r..
				if(p)
				   maj ? (p->major = r) : (p->minor = r);
				r->parent = p;
				x = r->major;
				
				// link q and r..
				r->minor = q;
				if(q)
				   q->parent = r;
				
				
				if(!n->parent)
					*head = r;
			}
			else
			{
				Node* a = r->parent;
				
				SP_ASSERT(a, "Expected minimum to have parent");
				Node* b = r->major;
				
				if(a == m)
				{
					// bst delete case 3.2
					// link parent and r..
					if(p)
						maj ? (p->major = r) : (p->minor = r);
					r->parent = n->parent;
					
					// link q and r..
					if(q)
					   q->parent = r;
					r->minor = q;
					
					// link m and r..
					r->major = m;
					m->parent = r;
					
					// link m and b..
					m->minor = b;
					if(b)
						b->parent = m;
					
					x = b;
					
					if(!n->parent)
						*head = r;
				}
				else
				{
					// bst delete case 3.3
					// link parent and r..
					if(p)
						maj ? (p->major = r) : (p->minor = r);
					r->parent = n->parent;
					
					// link q and r..
					r->minor = q;
					if(q)
					    q->parent = r;
					
					// link m and r..
					r->major = m;
					m->parent = r;
					
					// link a and b..
					a->minor = b;
					if(b)
						b->parent = a;
					x = b;
					if(!n->parent)
						*head = r;
				}
			}
			
			*replacement = r;
		}
			
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

bool procedure_1(Node** _x, Node** _w, Node** head)
{
	// x is black and w is red..
	Node* x = *_x;
	Node* w = *_w;
	
	if(!x)
	{
		return false;
	}
	
	Node dW;
	if(!w)
	{
		dW.parent = x->parent;
		dW.red = false;
		dW.major = dW.minor = NULL;
		w = &dW;
	}
	
	if(!x->red && w->red)
	{	
		SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
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
		 
		bool maj = !isMajor(w);
		
		// 1. colour w black..
		w->red = false;
		
		// 2. colour x's parent red..
		x->parent->red = true;
		/*
		 *	3. rotate x's parent:
		 *		if x is major, then rotate right..
		 *		if x is minor, then rotate left..
		 */
		Node* p = maj ? rotateRight(x->parent, head) : rotateLeft(x->parent, head);
		SP_ASSERT(p, "Expeceted to have replacement");
		p = maj ? p->major : p->minor;
		x = maj ? p->major : p->minor;
		
		
		// 4. set w to x's sibling..
		w = maj ? p->minor : p->major;
		
		//SP_ASSERT(w, "Expected to have sibling");
		Node n;
		if(!x)
		{
			n.parent = w ? w->parent : NULL;
			n.red = false;
			n.major = n.minor = NULL;
			x = &n;
		}
		
		Node m;
		if(!w)
		{
			m.parent = x ? x->parent : NULL;
			m.red = false;
			m.major = m.minor = NULL;
			w = &m;
		}
		
		// 5. decide case 2, 3 or 4 from this state..
		if(procedure_2(&x, &w, head))
			return true;
		
		if(procedure_3(&x, &w, head))
			return true;
		
		return procedure_4(&x, &w, head);
	}
	return false;
}

bool procedure_2(Node** _x, Node** _w, Node** head)
{	
	// x is black and w is black..
	Node* w = *_w;
	Node* x = *_x;
	if(!x && !w)
		return false;
	
	Node dX, dW;
	
	if(!w)
	{
		dW.parent = x->parent;
		dW.red = false;
		dW.minor = dW.major = NULL;
		w = &dW;
	}
	
	if(!x)
	{
		dX.parent = w->parent;
		dX.red = false;
		dX.minor = dX.major = NULL;
		x = &dX;
	}
	
	if(!x->red && !w->red)
	{
		SP_ASSERT(x->parent, "Impossible: this cannot be a root node");
		SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
		
		bool wMnB = w->minor ? !w->minor->red : true;
		bool wMjB = w->major ? !w->major->red : true;
		
		if(wMnB && wMjB)
		{
			/*
			 *	1. colour w red..
			 *	2. set x to its parent..
			 *		if x is red, colour x black (return)..
			 *		if x is black and root, return..
			 *		if x is black and not root, decide on case 1, 2, 3, 4..
			 */
			
			// 1. colour w red..
			w->red = false;
			
			// set x to its parent..
			x = x->parent;
			SP_ASSERT(x, "Replacement expected to have parent");
			
			bool xRed = x ? x->red : false;
			
			if(xRed)
			{
				// x is red, colour x black (return)..
				x->red = false;
				return true;
			}
			else
			{
				if(isRoot(x))
					return true;
				
				// x is black..
				// x is not root, decide on case 1, 2, 3, 4..
				w = isMajor(x) ? x->parent->minor : x->parent->major;
				if(procedure_1(&x, &w, head))
					return true;
				if(procedure_2(&x, &w, head))
					return true;
				if(procedure_3(&x, &w, head))
					return true;
				return procedure_4(&x, &w, head);
				
			}
		}
	}
	return false;
}

bool procedure_3(Node** _x, Node** _w, Node** head)
{
	// (w is black) and (isMajor(x) ? w's major is red and w's minor is black : w's minor is red and w's major is black) (hard check)..
	
	Node* x = *_x;
	Node* w = *_w;
	
	bool maj = !isMajor(w);
	bool wMjB = w->major ? !w->major->red : true;
	bool wMnB = w->minor ? !w->minor->red : true;
	
	if(maj ? (!wMjB && wMnB) : (!wMnB && wMjB))
	{
		SP_ASSERT((*_x)->parent == (*_w)->parent, "Replacement and sibling expected to have equal parent");
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
	 

		
		/*
		 *	1. colour w's child black:
		 *		if x is major, then colour w's major black..
		 *		if x is minor, then colour w's minor black..
		 */
		SP_ASSERT(maj ? w->major : w->minor, "Expected sibling child");
		maj ? (w->major->red = false) : (w->minor->red = false);
		
		// 2. colour w red..
		w->red = true;
		
		/*
		 *	3. rotate w:
		 *		if x is major, rotate left..
		 *		if x is minor, rotate right..
		 */
		w = maj ? rotateLeft(w, head) : rotateRight(w, head);
		
		// 4. set w to x's sibling..
		//w = maj ? (w->minor) : (w->major);
		SP_ASSERT(w != x, "Sibling cannot be the same");
		
		// 5. proceed to case 4..
		return procedure_4(_x, &w, head);
	}
	return false;
}

bool procedure_4(Node** _x, Node** _w, Node** head)
{
	// (w is black) and (isMajor(x) ? w's major is red : w's minor is red) (soft check)..
	
	Node* x = *_x;
	Node* w = *_w;
	bool maj = !isMajor(w);
	
	bool wMjR = w->major ? w->major->red : false;
	bool wMnR = w->minor ? w->minor->red : false;
	
	if(maj ? wMnR : wMjR)
	{
		SP_ASSERT((*_x)->parent == (*_w)->parent, "Replacement and sibling expected to have equal parent");
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
		
		
		// 1. w's colour is x's parent's colour..
		w->red = x->parent->red;
		
		// 2. x's parent is black..
		x->parent->red = false;
		
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
		maj ? rotateRight(x->parent, head) : rotateLeft(x->parent, head);
	}
	return true;
}

// fix up rules..
bool nodeFixupRules_RBT(Node* x, Node* w, Node** head)
{
	// x's parent is never NULL..
	// due to SBT, x always has a sibling..
	SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");

	if(x->red)
	{
		// procedure 0
		//SP_DEBUG("procedure 0");
		return procedure_0(x);
	}
	else
	{
		SP_ASSERT(w, "Expeceted sibling");
		bool maj = !isMajor(w);
		
		if(w->red)
		{ 
			//SP_DEBUG("procedure 1");
			return procedure_1(&x, &w, head);
		}
		else
		{
			bool wMjB = w->major ? !w->major->red : true;
			bool wMnB = w->minor ? !w->minor->red : true;
			
			if(wMnB && wMjB)
			{
				SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
				return procedure_2(&x, &w, head);
			}
			
			if(maj ? (!wMjB && wMnB) : (!wMnB && wMjB))
			{
				//SP_DEBUG("procedure 3");
				return procedure_3(&x, &w, head);
			}
			
			if(maj ? !wMnB : !wMjB)
			{
				//SP_DEBUG("procedure 4");
				return procedure_4(&x, &w, head);
			}
			
			SP_ASSERT(0, "Expected procedure");
		}
	}
	return false;
}


// fix up function..
// colouring..
void nodeFixup_RBT(bool rBefore, Node* r, Node* x, Node* w, Node** head)
{	
	if(!x && !w)
	{
		// the deleted node had no children..
		return;
	}
	
	Node dX;
	Node dW;
	if(!x)
	{
		dX.parent = w->parent;
		dX.minor = dX.major = NULL;
		dX.red = false;
		x = &dX;
	}
	
	if(!w)
	{
		dW.parent = x->parent;
		dW.minor = dW.major = NULL;
		dW.red = false;
		w = &dW;
	}
	
	bool rAfter = r ? r->red : false;
	
	// 1. rule: deleted node is red, and replcament is red or NULL => return..
	if(rBefore && (rAfter || !r))
	{
		return;
	}
	// 2. rule: deleted node is red and replacement is black and not NULL 
	// => colour replacement red and proceed to appropriate case..
	if(rBefore && !rAfter && r)
	{
		r->red = true;
		SP_ASSERT(nodeFixupRules_RBT(x, w, head), "Fix up failed");
		return;
	}
	
	// 3. rule: deleted node is black and replacement is red
	// => colour replacement black and return..
	if(!rBefore && rAfter)
	{
		SP_ASSERT(r, "Expeceted replacement");
		r->red = false;
		return;
	}
	
	// 4. rule: deleted node is black and replcaement is black..
	if(!rBefore && !rAfter)
	{
		// if replacement is not root, then proceed to appropriate case..
		if(x->parent)
		{
			SP_ASSERT(nodeFixupRules_RBT(x, w, head), "Fix up failed");
		}
		return;
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
void nodeDelete_RBT(Node** head, unsigned long long value)
{
	Node* n = nodeFind(*head, value);
	if(n)
	{
        bool aRed = n->red;
		bool bRed = false;
		Node* replacement = NULL;
        auto xw = nodeDeleteImpl_BST(n, head, &replacement);
		
		
		Node* r = NULL;
		Node* x = xw.first;
		Node* w = xw.second;
		if(x && w)
		{
			SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
		}
		//printf("intermediate form: (red before: %s, red after: %s) x: %c, w: %c\n", aRed ? "true" : "false", bRed ? "true" : "false", x ? ((x->red) ? 'R' : 'B') : 'B', w ? ((w->red) ? 'R' : 'B') : 'B');
		//if(*head) nodePrint(*head);
		nodeFixup_RBT(aRed, replacement, x, w, head);
	}
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