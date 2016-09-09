// ------------------------------------------------------------- //
// rnbw_tree.c: implements rnbw_trees                            //
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

#include"rnbw_tree.h"


void rnbw_node_struct::after_rot()
{
  // the parent gets the sub_weight of this node
  parent()->sub_weight = sub_weight;

  // recalculate the sub_weight field of this node
  sub_weight = weight;
  if(left_child()) sub_weight += left_child()->sub_weight;
  if(right_child()) sub_weight += right_child()->sub_weight;
}

void rnbw_node_struct::init()
{
  // initialize the sub_weight field of this node
  sub_weight = weight;
  if(left_child()) sub_weight += left_child()->sub_weight;
  if(right_child()) sub_weight += right_child()->sub_weight;
}

void rnbw_node_struct::isolate()
// We fix the sub_weight fields of the ancestors of this node.
{
  // fix sub_weight fields
  for(rnbw_node aux = parent(); aux; aux = aux->parent())
    aux->sub_weight -= sub_weight;

  // fix base class
  rnb_node_struct::isolate();
}

void rnbw_node_struct::set_weight(int w)
// sets the weight of this node to w
// Prec.: w >= 0
{
  // remember the difference between the new and the old weight
  int w_diff = w - weight;

  // update the weight and subweight fields of this node
  sub_weight += w_diff;
  weight = w;

  // update the sub_weight fields of the ancestors of this node
  for(rnbw_node aux = parent(); aux; aux = aux->parent())
    aux->sub_weight += w_diff;
}

void rnbw_node_struct::add_weight(int a)
// adds a to the weight of this node
// Prec.: a >= -(weight of this node)
{
  // update the weight and subweight fields of this node
  sub_weight += a;
  weight += a;

  // update the sub_weight fields of the ancestors of this node
  for(rnbw_node aux = parent(); aux; aux = aux->parent())
    aux->sub_weight += a;
}

rnbw_node rnbw_locate(rnbw_tree t, int w, int& offset)
// returns the node in the tree rooted at t which corresponds to
// w with respect to In-order.
// Prec.: 0 < w <= weight of tree rooted at t
{
  // current node
  rnbw_node curr_node = t;
  // sum of weights up to but excluding current node
  int lower = curr_node->left_child() ?
              curr_node->left_child()->sub_weight : 0;
  // sum of weights up to and including current node
  int upper  = lower + curr_node->weight;

  while(w <= lower || w > upper)
  // weight w not represented at current node
  // so we have to proceed at a child of the current node
  {
    if(w <= lower)
    // proceed at left child
    {
      curr_node = curr_node->left_child();
      // update lower
      lower -= curr_node->sub_weight;
      if(curr_node->left_child())
        lower += curr_node->left_child()->sub_weight;
      // update upper
      upper = lower + curr_node->weight;
    }
    else
    // proceed at right child
    {
      curr_node = curr_node->right_child();
      // update lower
      lower = upper + curr_node->sub_weight - curr_node->weight;
      if(curr_node->right_child())
        lower -= curr_node->right_child()->sub_weight;
      // update upper
      upper = lower + curr_node->weight;
    }
  }

  // store offset of w from lower
  offset = w - lower;

  // return the node representing w
  return curr_node;
}

void rnbw_node_struct::print()
// we redefine print in order to output also the additional fields
{
  // output base fields
  rnb_node_struct::print();

  // output new fields
  std::cout << "  weight:      " << weight << "\n";
  std::cout << "  sub_weight:  " << sub_weight << "\n";
}

