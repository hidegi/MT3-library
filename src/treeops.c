#include "internal.h"
#define MT3_MAX(a, b) ((a) > (b) ? (a) : (b))

static MT3_node _mt3_rotate_left(MT3_node n, MT3_node* head);
static MT3_node _mt3_rotate_right(MT3_node n, MT3_node* head);
static SPbool _mt3_transplant_rbt(MT3_node x, MT3_node w, MT3_node* head);
static SPbool _mt3_transplant_proc_0(MT3_node x);
static SPbool _mt3_transplant_proc_1(MT3_node x, MT3_node w, MT3_node* head);
static SPbool _mt3_transplant_proc_2(MT3_node x, MT3_node w, MT3_node* head);
static SPbool _mt3_transplant_proc_3(MT3_node x, MT3_node w, MT3_node* head);
static SPbool _mt3_transplant_proc_4(MT3_node x, MT3_node w, MT3_node* head);

static MT3_node _mt3_find_max(MT3_node n)
{
	return n ? (!n->major ? n : _mt3_find_max(n->major)) : NULL;
}

static MT3_node _mt3_find_min(MT3_node n)
{
	return n ? (!n->minor ? n : _mt3_find_min(n->minor)) : NULL;
}

SPbool _mt3_is_major(const MT3_node node)
{
	return (node && node->parent) ? (node->parent->major == node) : SP_FALSE;
}

SPbool _mt3_is_root(const MT3_node node)
{
	return (node && !node->parent);
}

static MT3_node _mt3_find_member(const MT3_node node)
{
	SP_ASSERT(!_mt3_is_root(node->parent), "Oopsie, this should not have happened");
	return _mt3_is_major(node->parent) ? node->parent->parent->minor : node->parent->parent->major;
}

static SPsize _mt3_calculate_black_depth(const MT3_node rbt)
{
	if(!rbt)
		return 1;

	SPsize count = 0;
	if(!rbt->red)
		count++;
	SPsize majorDepth = _mt3_calculate_black_depth(rbt->major);
	SPsize minorDepth = _mt3_calculate_black_depth(rbt->minor);
	return count + MT3_MAX(minorDepth, majorDepth);
}

static SPbool _mt3_verify_rbt_impl(const MT3_node rbt, SPsize depth, SPsize ref)
{
	if(rbt)
	{
		if(rbt->red)
		{
			if(!rbt->parent)
				return SP_FALSE;
			else
			{
				if(rbt->parent->red)
					return SP_FALSE;
			}
		}
		else
		{
			++depth;
		}

		if(rbt->major || rbt->minor)
		{
			return _mt3_verify_rbt_impl(rbt->major, depth, ref) && _mt3_verify_rbt_impl(rbt->minor, depth, ref);
		}
		else
		{
			return !rbt->red ? (ref == depth) : SP_TRUE;
		}
	}
	return SP_TRUE;
}

SPbool mt3_IsValidRBT(const MT3_node rbt)
{
	SPsize depth = _mt3_calculate_black_depth(rbt) - 1;
	return _mt3_verify_rbt_impl(rbt, 0, depth);
}

void _mt3_fix_rbt_violations(MT3_node node, MT3_node* head)
{
	if(node)
	{
		if(node->red && node->parent->red)
		{
			MT3_node m = _mt3_find_member(node);
			MT3_node p = node->parent;
			MT3_node g = p->parent;
			MT3_node r = NULL;

			SPbool isRed = m != NULL ? m->red : SP_FALSE;

			if(isRed)
			{
				// color switch
				p->red = !p->red;
				g->red = g->parent ? !g->red : SP_FALSE;
				m->red = !m->red;
				r = g;
			}
			else
			{
				/*
				 *  parent-child combinations:
				 *
				 *  major = true
				 *  minor = false
				 *
				 *  P   C | rotation
				 * -------|----------
				 *  0   0 |	R
				 *  0   1 |	LR
				 *  1   0 |	RL
				 *  1   1 |	L
				 *
				 *	for skews, rotate once
				 *	for triangles, rotate twice
				 */
				 SPbool pMajor = _mt3_is_major(p);
				 SPbool nMajor = _mt3_is_major(node);

				 if(!pMajor && !nMajor)
				 {
					// R-rotation
					 r = _mt3_rotate_right(node->parent->parent, head);
				 }
				 else if(!pMajor && nMajor)
				 {
					// LR-rotation
					r = _mt3_rotate_left(node->parent, head);
					    _mt3_rotate_right(node->parent, head);
				 }
				 else if(pMajor && !nMajor)
				 {
					// RL-rotation
					r = _mt3_rotate_right(node->parent, head);
					    _mt3_rotate_left(node->parent, head);
				 }
				 else
				 {
					// L-rotation
					r = _mt3_rotate_left(node->parent->parent, head);
				 }
				 r->red = !r->red;
				 pMajor ? (r->minor->red = !r->minor->red) : (r->major->red = !r->major->red);
			}
			_mt3_fix_rbt_violations(r, head);
		}
	}
}

static MT3_node _mt3_rotate_left(MT3_node n, MT3_node* head)
{
	SP_ASSERT(n, "Expected rotation node");
	SPbool maj = _mt3_is_major(n);
	MT3_node p = n->parent;
	MT3_node m = n->major;
	MT3_node c = m ? n->major->minor : NULL;

	n->parent = m;
	n->major->minor = n;

	if(c)
	   c->parent = n;
	n->major = c;

	if(m)
	   m->parent = p;

	p ? (maj ? (p->major = m) : (p->minor = m)) : (*head = m);
	return m;
}

static MT3_node _mt3_rotate_right(MT3_node n, MT3_node* head)
{
	SP_ASSERT(n, "Expected rotation node");
	SPbool maj = _mt3_is_major(n);
	MT3_node p = n->parent;
	MT3_node m = n->minor;
	MT3_node c = m ? n->minor->major : NULL;

	n->parent = m;
	n->minor->major = n;

	if(c)
	   c->parent = n;
	n->minor = c;

	if(m)
	   m->parent = p;

	p ? (maj ? (p->major = m) : (p->minor = m)) : (*head = m);
	return m;
}

void _mt3_bst_delete_impl(MT3_node n, MT3_node* head, MT3_node* _r, MT3_node* _x, MT3_node* _w)
{
	if(n)
	{
		MT3_node x = NULL;
		MT3_node w = NULL;
		MT3_node r = NULL;
		MT3_node p = n->parent;

		SPbool maj = _mt3_is_major(n);
		SPbool root = _mt3_is_root(n);

		if(!n->major && !n->minor)
		{
			if(p)
			{
				maj ? (p->major = NULL) : (p->minor = NULL);
				w = maj ? (p->minor) : (p->major);
			}
			else
			{
				SP_ASSERT(root, "Fatal, expected deleted node to be root");
			}
		}
		else if((n->major && !n->minor) || (!n->major && n->minor))
		{
			r = n->major ? n->major : n->minor;
			SP_ASSERT(r, "Fatal, expected to have replacement");
			r->parent = p;

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
		else
		{
#ifndef MT3_HAVE_BST_MAJOR_INCLINED
			r = _mt3_find_min(n->major);
			w = r->parent ? (_mt3_is_major(r) ? r->parent->minor : r->parent->major) : NULL;

			SP_ASSERT(r, "Expected to have replacement");
			SP_ASSERT(!r->minor, "Expected to have minimum replacement");
			MT3_node q = n->minor;
			MT3_node m = n->major;
			MT3_node a = r->parent;
			MT3_node b = r->major;

			SP_ASSERT(!r->minor, "Expected to have no more minimum");

			if(p)
			   maj ? (p->major = r) : (p->minor = r);
			r->parent = p;

			if(q)
			   q->parent = r;
			r->minor = q;

			if(r != m)
			{
				SP_ASSERT(a, "Expected minimum to have parent");

				// link m and r
				r->major = m;
				m->parent = r;

				// link a and b
				a->minor = b;
				if(b)
				   b->parent = a;
			}
#else
			r = _mt3_find_max(n->minor);
			w = r->parent ? (_mt3_is_major(r) ? r->parent->minor : r->parent->major) : NULL;
			SP_ASSERT(r, "Expected to have replacement");
			SP_ASSERT(!r->major, "Expected to maximum replacement");

			MT3_node q = n->major;
			MT3_node m = n->minor;
			MT3_node a = r->parent;
			MT3_node b = r->minor;

			SP_ASSERT(!r->major, "Expected to have no more maxima");
			if(p)
				maj ? (p->major = r) : (p->minor = r);
			r->parent = p;

			if(q)
			   q->parent = r;
			r->major = q;
			if(r != m)
			{
				// link m and r
				r->minor = m;
				m->parent = r;

				// link a and b
				a->major = b;
				if(b)
				   b->parent = a;
			}
#endif
			x = b;
		}

		*_r = r;
		*_x = x;
		*_w = w;

		if(!p)
		   *head = r;

		_mt3_delete_node(n);
	}
}

SPbool _mt3_fix_up_rbt(SPbool redBefore, MT3_node r, MT3_node x, MT3_node w, MT3_node* head)
{
	SPbool redAfter = r ? r->red : SP_FALSE;
	if(redBefore && redAfter)
		return SP_TRUE;

	if(!redBefore && redAfter)
	{
		if(r)
		   r->red = SP_FALSE;
	   return SP_TRUE;
	}

	if(redBefore && !redAfter)
	{
		if(r)
		   r->red = SP_TRUE;
	}

	if(_mt3_is_root(x))
		return SP_TRUE;

	return _mt3_transplant_rbt(x, w, head);
}

static SPbool _mt3_transplant_rbt(MT3_node x, MT3_node w, MT3_node* head)
{
	SPbool xRed = x ? x->red : SP_FALSE;
	if(xRed)
	{
		return _mt3_transplant_proc_0(x);
	}
	else
	{
		if(!w)
		{
			// following cases would expect w to have children
	        // since double black cannot have children, return here
			return SP_TRUE;
		}

		SPbool maj = !_mt3_is_major(w);
		if(w->red)
		{
			// x is black and w is red
			return _mt3_transplant_proc_1(x, w, head);
		}
		else
		{
			SPbool wMjB = w->major ? !w->major->red : SP_TRUE;
			SPbool wMnB = w->minor ? !w->minor->red : SP_TRUE;

			if(wMjB && wMnB)
			{
				return _mt3_transplant_proc_2(x, w, head);
			}

			SPbool c = maj ? (!wMjB && wMnB) : (!wMnB && wMjB);
			if(c)
			{
				return _mt3_transplant_proc_3(x, w, head);
			}

			c = maj ? !wMnB : !wMjB;
			if(c)
			{
				return _mt3_transplant_proc_4(x, w, head);
			}
		}
	}

	return SP_FALSE;
}

static SPbool _mt3_transplant_proc_0(MT3_node x)
{
	if(x->red)
	{
		x->red = SP_FALSE;
		return SP_TRUE;
	}

	return SP_FALSE;
}

static SPbool _mt3_transplant_proc_1(MT3_node x, MT3_node w, MT3_node* head)
{
	if(w)
	{
		SPbool xBlack = x ? !x->red : SP_TRUE;
		SPbool maj = !_mt3_is_major(w);
		if(xBlack && w->red)
		{
			// only x could be double black, w must be red
			SP_ASSERT(w->parent, "Replacement expected to have parent");
            		SP_ASSERT(maj ? w->parent->minor == w : w->parent->major == w, "Linking error");
			if(x)
            		{
                		SP_ASSERT(w->parent == x->parent, "Replacement and sibling expected to have equal parent");
            		}

			w->red = SP_FALSE;
			w->parent->red = SP_TRUE;
			MT3_node m = maj ? w->major : w->minor;
			MT3_node p = maj ? _mt3_rotate_right(w->parent, head) : _mt3_rotate_left(w->parent, head);
			SP_ASSERT(p, "Expeceted to have rotation replacement");
            		SP_ASSERT(p == w, "Rotation error");
			p = maj ? p->major : p->minor;
			SP_ASSERT(p, "Expected to have parent");
			x = maj ? p->major : p->minor;
			w = maj ? p->minor : p->major;
			SP_ASSERT(m == w, "Rotation error");
			return _mt3_transplant_rbt(x, w, head);
		}
	}
	return SP_FALSE;
}

static SPbool _mt3_transplant_proc_2(MT3_node x, MT3_node w, MT3_node* head)
{
	if(w)
	{
		// x is black and w is black
		// x could be double black, w cannot be double black
		SPbool xBlack = x ? !x->red : SP_TRUE;
		if(xBlack && !w->red)
		{
			// only x could be double black
			SPbool maj = !_mt3_is_major(w);
			SPbool wMnB = w->minor ? !w->minor->red : SP_TRUE;
			SPbool wMjB = w->major ? !w->major->red : SP_TRUE;
			if(wMnB && wMjB)
			{
				w->red = SP_TRUE;
				x = w->parent;
				if(_mt3_is_root(x))
				{
					x->red = SP_FALSE;
					return SP_TRUE;
				}
				maj = _mt3_is_major(x);
				w = maj ? x->parent->minor : x->parent->major;
				SP_ASSERT(x, "Replacement expected to have parent");
				if(x->red)
				{
					x->red = SP_FALSE;
					return SP_TRUE;
				}
				else
				{
					w = maj ? w->parent->minor : w->parent->major;
					x = maj ? w->parent->major : w->parent->minor;
					return _mt3_transplant_rbt(x, w, head);
				}
			}
		}
	}
	return SP_FALSE;
}

static SPbool _mt3_transplant_proc_3(MT3_node x, MT3_node w, MT3_node* head)
{
	if(w)
	{
		SPbool xBlack = x ? !x->red : SP_TRUE;
		SPbool wMjB = w->major ? !w->major->red : SP_TRUE;
		SPbool wMnB = w->minor ? !w->minor->red : SP_TRUE;
		SPbool maj = !_mt3_is_major(w);
		SPbool c = maj ? (!wMjB && wMnB) : (wMjB && !wMnB);

		if(xBlack && c)
		{
			if(x)
			{
				SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
			}
			maj = !_mt3_is_major(w);
			SP_ASSERT(maj ? w->major : w->minor, "Expected sibling's child");
			maj ? (w->major->red = SP_FALSE) : (w->minor->red = SP_FALSE);
			w->red = SP_TRUE;
			w = maj ? _mt3_rotate_left(w, head) : _mt3_rotate_right(w, head);
			SP_ASSERT(w, "Expected sibling");
            		SP_ASSERT(w->parent, "Expected parent");
			x = maj ? w->parent->major : w->parent->minor;
			SP_ASSERT(w != x, "Sibling and replacement cannot be the same");
			return _mt3_transplant_proc_4(x, w, head);
		}
	}
	return SP_FALSE;
}

static SPbool _mt3_transplant_proc_4(MT3_node x, MT3_node w, MT3_node* head)
{
	if(w)
	{
		// x can be double black
		// w cannot be double black
		SPbool wMjR = w->major ? w->major->red : SP_FALSE;
		SPbool wMnR = w->minor ? w->minor->red : SP_FALSE;
		SPbool xBlack = x ? !x->red : SP_TRUE;
		SPbool maj = !_mt3_is_major(w);
		SPbool c = maj ? wMnR : wMjR;
		
		if(xBlack && c)
		{
			if(x)
            		{
                		SP_ASSERT(x->parent == w->parent, "Replacement and sibling expected to have equal parent");
            		}
			SP_ASSERT(w->parent, "Expected to have parent");
			w->red = w->parent->red;
			w->parent->red = SP_FALSE;
			SP_ASSERT(maj ? w->minor : w->major, "Expected sibling child");
			maj ? (w->minor->red = SP_FALSE) : (w->major->red = SP_FALSE);
			maj ? _mt3_rotate_right(w->parent, head) : _mt3_rotate_left(w->parent, head);
			return SP_TRUE;
		}
	}
	return SP_FALSE;
}