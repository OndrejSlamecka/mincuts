// ------------------------------------------------------------- //
// ed_tree.h: declaration of ed_trees. An ed_tree stores the     //
//            non-tree edges adjacent to a node at a certain     //
//            level in the dynamic connectivity algorithm by     //
//            M. Rauch Henzinger and V. King                     //
//                                                               //
// Version 1.3                                                   //
//                                                               //
// Copyright (C) 1995 by David Alberts                           //
// FU Berlin, Inst. f. Informatik, Takustr. 9, 14195 Berlin, FRG //
// All rights reserved.                                          //
// There is ABSOLUTELY NO WARRANTY.                              //
// ------------------------------------------------------------- //

// RCS Id //
/* $Id: dyn_con.fw,v 1.17 1995/11/22 10:13:55 alberts Exp $ */

#ifndef ED_TREE
#define ED_TREE

#include <ogdf/basic/Graph.h>
#include "rnbw_tree.h"


class   ed_node_struct;
typedef ed_node_struct* ed_node;
typedef ed_node ed_tree;

class ed_node_struct : public rnbw_node_struct {

public:

  ed_node_struct(ogdf::edge e) : rnbw_node_struct(1)   // constructor
  {                        // each node contains exactly one edge
    ed_edge = e;
  }

  // --- Conversion Functions ---
  ed_node left_child() { return (ed_node) child[rnb_left]; }
  ed_node right_child() { return (ed_node) child[rnb_right]; }
  friend inline ed_tree ed_join(ed_tree t1, ed_tree t2, ed_node dummy)
  { return (ed_tree) rnb_join(t1,t2,dummy); }
  friend inline ed_node ed_locate(ed_tree edt, int w, int& offset)
  { return (ed_node) rnbw_locate(edt,w,offset); }

  ogdf::edge get_corr_edge() { return ed_edge; }
  // this ed_node corresponds to the returned edge of the graph

  friend ed_node ed_insert(ed_tree& edt, ogdf::edge e, ed_node dummy);
  // create a new node for e and insert it into the tree edt
  // the new root of the tree is stored in edt
  // the new node is returned

  friend void ed_delete(ed_tree& edt, ed_node edn, ed_node dummy);
  // delete the node edn in the ed_tree edt
  // the new root is stored in edt

  void print();
  // prints this node to the screen, for testing

private:

  ogdf::edge ed_edge;    // corresponding edge in G
};


#endif
