// ------------------------------------------------------------- //
// rnbw_tree.h: header file for the rnbw_trees                   //
//                                                               //
// comment: rnbw_trees are derived from rnb_trees. They have an  //
//          additional non-negative weight at each node and      //
//          subtree weights.                                     //
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

#ifndef RNBW_TREE
#define RNBW_TREE

#include"rnb_tree.h"

class rnbw_node_struct;
typedef rnbw_node_struct* rnbw_node;
typedef rnbw_node rnbw_tree;


class rnbw_node_struct : public rnb_node_struct {

public:

  rnbw_node_struct(int w = 1) : rnb_node_struct() { weight = sub_weight = w; }
  // construct a new tree containing just one node with weight w.
  // By default each node gets a weight of one.
  // Prec.: w >= 0

  int get_weight() { return weight; }
  // returns the weight of this node

  int get_subtree_weight() { return sub_weight; }
  // returns the weight of the subtree rooted at this node

  void set_weight(int w);
  // sets the weight of this node to w
  // Prec.: w >= 0

  void add_weight(int a);
  // adds a to the weight of this node
  // Prec.: a >= -(weight of this node)

  friend rnbw_node rnbw_locate(rnbw_tree t, int w, int& offset);
  // returns the node in the tree rooted at t which corresponds to
  // w with respect to In-order.
  // Prec.: 0 < w <= weight of tree rooted at t

  // --- Conversion Functions ---
  rnbw_node parent() { return (rnbw_node) par; }
  rnbw_node left_child() { return (rnbw_node) child[rnb_left]; }
  rnbw_node right_child() { return (rnbw_node) child[rnb_right]; }
  rnbw_tree find_root() {return (rnbw_tree) rnb_node_struct::find_root(); }
  rnbw_node pred() { return (rnbw_node) rnb_node_struct::pred(); }
  rnbw_node succ() { return (rnbw_node) rnb_node_struct::succ(); }
  friend inline rnbw_tree rnbw_join(rnbw_tree t1, rnbw_tree t2, rnbw_node dummy)
  { return (rnbw_tree) rnb_join(t1,t2,dummy); }

  virtual void print();
  // we redefine print in order to output also the additional fields

protected:

  void after_rot();
  // We fix the weight fields after a rotation. This is called as
  // a virtual function in the base class rnb_tree.

  void init();
  // This method is used to initialize the dummy node in join and split
  // after linking it to the tree(s). It is a virtual function in the base
  // class.

  virtual void isolate();
  // We fix the sub_weight fields of the ancestors of this node.

private:

  int weight;      // stores the weight of this node
  int sub_weight;  // stores the weight of the subtree rooted at this node
};

#endif
