// ------------------------------------------------------------- //
// rnb_tree.h: header file for rnb_trees                         //
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

// RCS ID //
/* $Id: dyn_con.fw,v 1.17 1995/11/22 10:13:55 alberts Exp $ */

#ifndef RNB_TREE  // avoid multiple inclusion
#define RNB_TREE

#include <iostream>
#include <random>

// we define left and right
enum rnb_dir {rnb_left=0, rnb_right=1};

class   rnb_node_struct;
typedef rnb_node_struct* rnb_node;
typedef rnb_node rnb_tree;

class rnb_node_struct{

public:

  rnb_node_struct() {
      par = child[0] = child[1] = nullptr;
      prio = uniform(random_generator);
  }
  // construct a new tree containing just one element

  virtual ~rnb_node_struct() { isolate(); }
  // virtual destructor in order to deallocate the right amount of storage
  // even in derived classes

  rnb_node find_root();
  // returns the root of the tree containing this node.
  // Prec.: this != nullptr

  rnb_node sub_pred();
  // returns the predecessor of this node in the (sub)tree rooted at this node
  // or nullptr if it does not exist
  // Prec.: this != nullptr

  rnb_node sub_succ();
  // returns the successor of this node in the (sub)tree rooted at this node
  // or nullptr if it does not exist
  // Prec.: this != nullptr

  rnb_node pred();
  // returns the predecessor of this node or nullptr if it does not exist
  // Prec.: this != nullptr

  rnb_node succ();
  // returns the successor of this node or nullptr if it does not exist
  // Prec.: this != nullptr

  rnb_node cyclic_pred() { return (this == first()) ? last() : pred(); }
  // return the cyclic predecessor of this node (or nullptr)
  // Prec.: this != nullptr

  rnb_node cyclic_succ() { return (this == last()) ? first() : succ(); }
  // return the cyclic successor of this node (or nullptr)
  // Prec.: this != nullptr

  rnb_node first();
  // Return the first node in In-order in the tree rooted at this node.

  rnb_node last();
  // Return the last node of this tree.

  friend int smaller(rnb_node u, rnb_node v);
  // returns true iff u is smaller than v

  friend rnb_tree rnb_join(rnb_tree t1, rnb_tree t2, rnb_node dummy);
  // join t1 and t2 and return the resulting rnb_tree

  friend void split(rnb_node at, int where, rnb_tree& t1, rnb_tree& t2,
                    rnb_node dummy);
  // split the rnb_tree containing the node at before or after at
  // depending on where. If where == rnb_left we split before at,
  // else we split after at. The resulting trees are stored in t1 and t2.
  // If at == nullptr, we store nullptr in t1 and t2.

  virtual void print();
  // prints the contents of this node to stdout for testing

  friend void traverse(rnb_tree t);
  // traverses the tree t and prints each node to stdout for testing

protected:

  rnb_node  par;                 // parent node
  rnb_node  child[2];            // children
  long      prio;                // priority for balancing

  friend void rotate(rnb_node rot_child, rnb_node rot_parent);
  // Rotate such that rot_child becomes the parent of rot_parent.
  // Prec.: rot_child is a child of rot_parent

  virtual void after_rot() { }
  // This method is called for rot_parent after each rotation in order
  // to fix additional information at the nodes in derived classes.

  virtual void init() { }
  // This method is used to initialize the dummy node in join and split
  // after linking it to the tree(s).
  // Prec.: this != nullptr

  virtual void isolate();
  // Make this node an isolated node.
  // Prec.: this != nullptr

  thread_local static std::mt19937 random_generator;  // for computing random priorities
  thread_local static std::uniform_int_distribution<std::mt19937::result_type> uniform;
};

#endif
