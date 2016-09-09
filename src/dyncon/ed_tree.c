// ------------------------------------------------------------- //
// ed_tree.c: implementation of ed_trees.                        //
//                                                               //
// Version 1.3                                                   //
//                                                               //
// Copyright (C) 1995 by David Alberts                           //
// FU Berlin, Inst. f. Informatik, Takustr. 9, 14195 Berlin, FRG //
// All rights reserved.                                          //
// There is ABSOLUTELY NO WARRANTY.                              //
// ------------------------------------------------------------- //

#include"ed_tree.h"


ed_node ed_insert(ed_tree& edt, ogdf::edge e, ed_node dummy)
// create a new node for e and insert it into the tree edt
// the new root of the tree is stored at edt
// the new node is returned
{
  ed_tree aux = new ed_node_struct(e);
  edt = ed_join(edt,aux,dummy);
  return aux;
}

void ed_delete(ed_tree& edt, ed_node edn, ed_node dummy)
// delete the node edn in the ed_tree edt
// the new root is stored in edt
{
  // split off edn
  rnb_tree t1,t2,t3;
  split(edn,rnb_left,t1,t2,dummy);
  split(edn,rnb_right,t3,t2,dummy);

  // now t3 contains just edn so we can safely delete edn
  delete edn;

  // merge the remaining pieces together again
  edt = ed_join((ed_tree)t1,(ed_tree)t2,dummy);
}

void ed_node_struct::print()
// we redefine print in order to output also the additional fields
{
  // output base fields
  rnbw_node_struct::print();

  // output new fields
  std::cout << "  ed_edge:     " << ed_edge << "\n";
}

