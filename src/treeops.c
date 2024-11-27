#include "internal.h"
#define MT3_MAX(a, b) ((a) > (b) ? (a) : (b))

static MT3_node _mt3_rotate_left(MT3_node n, MT3_node* head);
static MT3_node _mt3_rotate_right(MT3_node n, MT3_node* head);
/*
static void _mt3_bst_delete_impl(MT3_node* n, MT3_node* head, MT3_node** x, MT3_node** w, MT3_node** r);
static SPbool _mt3_is_major(const MT3_node* node);
static SPbool _mt3_is_root(const MT3_node* node);
static SPbool _mt3_fix_up_rbt(SPbool rBefore, MT3_node* r, MT3_node* x, MT3_node* w, MT3_node* head);
static SPbool _mt3_transplant_rbt(MT3_node* x, MT3_node* w, MT3_node* head);
static SPbool _mt3_transplant_proc_0(MT3_node* x);
static SPbool _mt3_transplant_proc_1(MT3_node* x, MT3_node* w, MT3_node* head);
static SPbool _mt3_transplant_proc_2(MT3_node* x, MT3_node* w, MT3_node* head);
static SPbool _mt3_transplant_proc_3(MT3_node* x, MT3_node* w, MT3_node* head);
static SPbool _mt3_transplant_proc_4(MT3_node* x, MT3_node* w, MT3_node* head);
*/
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

SPbool mt3_VerifyRBT(const MT3_node rbt)
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
				// color switch..
				p->red = !p->red;
				g->red = g->parent ? !g->red : SP_FALSE;
				m->red = !m->red;
				r = g;
			}
			else
			{
				/*
				 *  child-parent combinations:
				 *
				 *  major = true..
				 *  minor = false
				 *
				 *  P   C | rotation
				 * -------|----------
				 *  0   0 |	R
				 *  0   1 |	LR
				 *  1   0 |	RL
				 *  1   1 |	L
				 *
				 *	for skews, rotate once..
				 *	for triangles, rotate twice..
				 */
				 SPbool pMajor = _mt3_is_major(p);
				 SPbool nMajor = _mt3_is_major(node);

				 if(!pMajor && !nMajor)
				 {
					// R-rotation..
					 r = _mt3_rotate_right(node->parent->parent, head);
				 }
				 else if(!pMajor && nMajor)
				 {
					// LR-rotation..
					r = _mt3_rotate_left(node->parent, head);
					    _mt3_rotate_right(node->parent, head);
				 }
				 else if(pMajor && !nMajor)
				 {
					// RL-rotation..
					r = _mt3_rotate_right(node->parent, head);
					    _mt3_rotate_left(node->parent, head);
				 }
				 else
				 {
					// L-rotation..
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
