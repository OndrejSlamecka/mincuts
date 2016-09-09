// ------------------------------------------------------------- //
// rnb_tree.c: implementation of rnb_trees                       //
//                                                               //
// comment: rnb_tree is an implementation of balanced binary     //
//          trees with a randomized balancing scheme.            //
//          See also the documentation in dyn_con.ps.            //
//                                                               //
// Version 1.3                                                   //
//                                                               //
// Copyright (C) 1995 by David Alberts                           //
// FU Berlin, Inst. f. Informatik, Takustr. 9, 14195 Berlin, FRG //
// All rights reserved.                                          //
// There is ABSOLUTELY NO WARRANTY.                              //
// ------------------------------------------------------------- //

#include"rnb_tree.h"

std::mt19937 rnb_node_struct::random_generator;  // define the static member
std::uniform_int_distribution<std::mt19937::result_type>
  rnb_node_struct::uniform(std::numeric_limits<int>::min(),
    std::numeric_limits<int>::max());

inline void rotate(rnb_node  rot_child, rnb_node rot_parent)
// Rotate such that rot_child becomes the parent of rot_parent.
// Prec.: rot_child is a child of rot_parent.
{
  // determine the direction dir of the rotation
  int dir = (rot_parent->child[rnb_left] == rot_child) ? rnb_right : rnb_left;

   // subtree which changes sides
  rnb_tree middle = rot_child->child[dir];

  // fix middle tree
  rot_parent->child[1-dir] = middle;
  if(middle) middle->par = rot_parent;

  // fix parent field of rot_child
  rot_child->par = rot_parent->par;
  if(rot_child->par) {
    if(rot_child->par->child[rnb_left] == rot_parent) {
       rot_child->par->child[rnb_left]  = rot_child;
    } else {
       rot_child->par->child[rnb_right] = rot_child;
    }
  }

  // fix parent field of rot_parent
  rot_child->child[dir] = rot_parent;
  rot_parent->par = rot_child;

  // fix additional information in derived classes
  rot_parent->after_rot();
}

void rnb_node_struct::isolate()
// Make this node an isolated node.
// Prec.: this != nullptr
{
  // adjust child pointer of parent if it exists
  if(par)
  {
    if(par->child[rnb_left] == this) {
      par->child[rnb_left] = nullptr;
    } else {
      par->child[rnb_right] = nullptr;
    }
    par = nullptr;
  }

  // adjust parent pointers of children if they exist
  if(child[rnb_left])
  {
    child[rnb_left]->par = nullptr;
    child[rnb_left] = nullptr;
  }
  if(child[rnb_right])
  {
    child[rnb_right]->par = nullptr;
    child[rnb_right] = nullptr;
  }
}

rnb_tree rnb_node_struct::find_root()
// returns the root of the tree containing this node.
// Prec.: this != nullptr
{
  rnb_node aux;
  for(aux = this; aux->par; aux = aux->par);
  return aux;
}

rnb_node rnb_node_struct::sub_pred()
// returns the predecessor of this node in the subtree rooted at this node
// or nullptr if it does not exist
// Prec.: this != nullptr
{
  // handle the nullptr case first
  if(!child[rnb_left]) return nullptr;

  // find the last node with no right child in the left subtree of u
  rnb_node aux;
  for(aux = child[rnb_left]; aux->child[rnb_right];
      aux = aux->child[rnb_right]);
  return aux;
}

rnb_node rnb_node_struct::sub_succ()
// returns the successor of this node in the subtree rooted at this node
// or nullptr if it does not exist
// Prec.: this != nullptr
{
  // handle the nullptr case first
  if(!child[rnb_right]) return nullptr;

  // find the first node with no left child in the right subtree of u
  rnb_node aux;
  for(aux = child[rnb_right]; aux->child[rnb_left];
      aux = aux->child[rnb_left]);
  return aux;
}

rnb_node rnb_node_struct::pred()
// returns the predecessor of this node or nullptr if it does not exist
// Prec.: this != nullptr
{
  // search for predecessor in the subtree of this node first
  rnb_node sub_pr = sub_pred();
  // if it exists we can return it
  if(sub_pr) return sub_pr;

  // otherwise we have to look for the ancestors of this node
  if(par) {      // if there is a parent
    if(this == par->child[rnb_right]) {
    // this is a right child
      return par;
    } else {
    // this is a left child
      for(rnb_node aux = par; aux->par; aux = aux->par) {
        if(aux == aux->par->child[rnb_right]) {
          return aux->par;
        }
      }
    }
  }

  // there is no predecessor
  return nullptr;
}

rnb_node rnb_node_struct::succ()
// returns the successor of this node or nullptr if it does not exist
// Prec.: this != nullptr
{
  // search for successor in the subtree of this node first
  rnb_node sub_s = sub_succ();
  // if it exists we can return it
  if(sub_s) return sub_s;

  // otherwise we have to look for the ancestors of this node
  if(par) {      // if there is a parent of u
    if(this == par->child[rnb_left]) {
    // this node is a left child
      return par;
    } else {
    // this node is a right child
      for(rnb_node aux = par; aux->par; aux = aux->par)
        if(aux == aux->par->child[rnb_left]) return aux->par;
    }
  }

  // there is no predecessor
  return nullptr;
}

rnb_node rnb_node_struct::first()
// Return the first node in In-order in the tree rooted at this node.
{
  // remember one node before current node
  rnb_node last = nullptr;
  for(rnb_node current = this; current; current = current->child[rnb_left])
    last = current;

  return last;
}

rnb_node rnb_node_struct::last()
// Return the last node of this tree.
{
  // remember one node before current node
  rnb_node last = nullptr;
  for(rnb_node current = this; current; current = current->child[rnb_right])
    last = current;

  return last;
}

int smaller(rnb_node u, rnb_node v)
// returns true iff u is smaller than v
{
  if(!u || !v) return false;
  if(u == v) return false;

  // determine the height of u and v
  rnb_node aux_u = u;
  int u_height, v_height;
  for(u_height = 0; aux_u->par; aux_u = aux_u->par, u_height++);
  rnb_node aux_v = v;
  for(v_height = 0; aux_v->par; aux_v = aux_v->par, v_height++);

  // if u and v have different roots they are incomparable and we return false
  if(aux_u != aux_v) return false;

  // we represent the paths from u and v to their roots by arrays
  // create arrays
  int *u_path = new int[u_height];
  int *v_path = new int[v_height];

  // insert left and right moves
  int u_i = u_height - 1;
  for(aux_u = u; aux_u->par; aux_u = aux_u->par, u_i--)
  {
    if(aux_u->par->child[rnb_left] == aux_u) u_path[u_i] = rnb_left;
    else                                     u_path[u_i] = rnb_right;
  }

  int v_i = v_height - 1;
  for(aux_v = v; aux_v->par; aux_v = aux_v->par, v_i--)
  {
    if(aux_v->par->child[rnb_left] == aux_v) v_path[v_i] = rnb_left;
    else                                     v_path[v_i] = rnb_right;
  }

  // compare the paths
  // skip identical prefix
  int i;
  for(i = 0; ((i<u_height) && (i<v_height)) && (u_path[i] == v_path[i]);
      i++);

  // at least one path is not completely scanned because u!=v
  // but u->find_root() == v->find_root()
  // at i they are different
  int result;
  if( (i<u_height) && (u_path[i] == rnb_left) )
    result = true;
  else
    if( (i<v_height) && (v_path[i] == rnb_right) )
      result = true;
    else
      result = false;

  // delete the paths
  delete[] u_path;
  delete[] v_path;

  return result;
}

rnb_tree rnb_join(rnb_tree t1, rnb_tree t2, rnb_node dummy)
// join t1 and t2 and return the resulting rnb_tree
{
  // handle the trivial t1 == nullptr || t2 == nullptr case
  if(!t1 || !t2)
  {
    if(t1) return t1;
    if(t2) return t2;
    return nullptr;
  }

  dummy->par = nullptr;
  dummy->child[rnb_left] = t1;
  dummy->child[rnb_right] = t2;
  t1->par = dummy;
  t2->par = dummy;
  // fix additional information in derived classes
  dummy->init();

  // trickle dummy down
  while( (dummy->child[rnb_left]) || (dummy->child[rnb_right]) )
  // while there is at least one child
  {
    // rotate with child with biggest priority

    // find child with biggest priority...
    rnb_node bigger = dummy->child[rnb_left];
    if(dummy->child[rnb_right])
    {
      if(dummy->child[rnb_left])
      {
        if(dummy->child[rnb_right]->prio > dummy->child[rnb_left]->prio)
          bigger = dummy->child[rnb_right];
      }
      else bigger = dummy->child[rnb_right];
    }

    // ...and rotate with it
    rotate(bigger,dummy);
  }

  // disconnect dummy from the new tree
  dummy->isolate();

  // return root of the new tree
  if(t2->par) return t1;
  else        return t2;
}

void split(rnb_node at, int where, rnb_tree& t1, rnb_tree& t2, rnb_node dummy)
// split the rnb_tree containing the node at before or after at
// depending on where. If where == rnb_left we split before at,
// else we split after at. The resulting trees are stored in t1 and t2.
// If at == nullptr, we store nullptr in t1 and t2.
{
  // handle the trivial at == nullptr case first
  if(!at)
  {
    t1 = nullptr;
    t2 = nullptr;
    return;
  }

  dummy->child[rnb_left] = nullptr;
  dummy->child[rnb_right] = nullptr;

  // insert dummy in the right place (w.r.t. In-order)
  // where == rnb_left => split before at
  // where != rnb_left => split after at
  if(where != rnb_left) // split after at
  {
    // store dummy as left child of the subtree successor of at
    // or as right child of at if there is no subtree successor
    rnb_node s = at->sub_succ();
    if(!s)
    {
      at->child[rnb_right] = dummy;
      dummy->par = at;
    }
    else
    {
      s->child[rnb_left] = dummy;
      dummy->par = s;
    }
  }
  else     // split before at
  {
    // store dummy as right child of the subtree predecessor of at
    // or as left child of at if there is no subtree predecessor
    rnb_node p = at->sub_pred();
    if(!p)
    {
      at->child[rnb_left] = dummy;
      dummy->par = at;
    }
    else
    {
      p->child[rnb_right] = dummy;
      dummy->par = p;
    }
  }
  // fix additional information in derived classes
  dummy->init();


  // rotate dummy up until it becomes the root
  for(rnb_node u = dummy->par; u; u = dummy->par) rotate(dummy,u);

  // store the subtrees of dummy in t1 and t2
  t1 = dummy->child[rnb_left];
  t2 = dummy->child[rnb_right];

  // disconnect dummy
  dummy->isolate();
}

void rnb_node_struct::print()
// prints the contents of this node to stdout for testing
// Prec.: this != nullptr
{
  std::cout << "node at " << this << ":\n";
  std::cout << "  parent:      " << par << "\n";
  std::cout << "  left child:  " << child[rnb_left] << "\n";
  std::cout << "  right child: " << child[rnb_right] << "\n";
  std::cout << "  priority:    " << prio << "\n";
}

void traverse(rnb_tree t)
// traverses the tree and outputs each node to stdout for testing
{
  if(t)
  {
    t->print();
    traverse(t->child[rnb_left]);
    traverse(t->child[rnb_right]);
  }
}

