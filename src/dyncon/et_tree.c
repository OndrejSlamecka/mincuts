// ------------------------------------------------------------- //
// et_tree.c: implementation of et_trees.                        //
//                                                               //
// Version 1.3                                                   //
//                                                               //
// Copyright (C) 1995 by David Alberts                           //
// FU Berlin, Inst. f. Informatik, Takustr. 9, 14195 Berlin, FRG //
// All rights reserved.                                          //
// There is ABSOLUTELY NO WARRANTY.                              //
// ------------------------------------------------------------- //

#include "dyn_con.h"

using ogdf::node;
using ogdf::edge;
using std::cout;

et_node_struct::et_node_struct
(dyn_con* dcp, node v, int my_level, int activate)
: rnbw_node_struct(0)
{
  dc = dcp;
  corr_node = v;
  level = my_level;
  active = activate;
  if(activate) dc->Gp->act_occ[v][level] = this;
  edge_occ[0] = edge_occ[1] = nullptr;
}

et_node_struct::et_node_struct(et_node en)
: rnbw_node_struct(0)
{
  dc = en->dc;
  corr_node = en->corr_node;
  level = en->level;
  active = false;
  edge_occ[0] = edge_occ[1] = nullptr;
}

void et_node_struct::pass_activity(et_node to)
// Make this node inactive and pass its activity to to.
// Prec.: this node is active, to represents the same vertex and is on the
//        same level.
{
  active = false;
  to->active = true;
  to->set_weight(get_weight());
  set_weight(0);
  dc->Gp->act_occ[corr_node][level] = to;
}

void change_root(et_tree& et, et_node en, int i, dyn_con* dc)
// Change the root of the tree T represented by the et_tree et to the
// vertex represented by the et_node en. The new tree is stored at et.
// Prec.: The et_node en is in the et_tree et.
{
  et_node first_nd = et->first();

  // if en is already the first node do nothing
  if(en == first_nd) return;

  // create a new occurrence for the new root
  et_node new_occ = new et_node_struct(en);

  // --- update active occurrences --- //
  et_node last_nd = et->last();
  // if the first node is active, pass activity to last node, since
  // the first node will be deleted
  if(first_nd->active) first_nd->pass_activity(last_nd);

  // --- update tree_occs --- //
  if(en->edge_occ[rnb_left] == en->edge_occ[rnb_right])
  {
    // replace the nullptr pointer in tree_occs of en->edge_occ[rnb_left]
    // by the new occurrence
    int k;
    for(k=0; nullptr != dc->Gp->tree_occ[en->edge_occ[rnb_left]][i][k]; k++);
    dc->Gp->tree_occ[en->edge_occ[rnb_left]][i][k] = new_occ;
  }
  else
  {
    // replace en by the new occurrence
    int k;
    for(k=0; en != dc->Gp->tree_occ[en->edge_occ[rnb_left]][i][k]; k++);
    dc->Gp->tree_occ[en->edge_occ[rnb_left]][i][k] = new_occ;
  }

  edge first_edge = first_nd->edge_occ[rnb_right];
  if((first_edge != last_nd->edge_occ[rnb_left]) ||
     (en == last_nd))
  {
    // replace first_nd by last_nd in the tree_occs of first_edge
    int k;
    for(k=0; first_nd != dc->Gp->tree_occ[first_edge][i][k]; k++);
    dc->Gp->tree_occ[first_edge][i][k] = last_nd;
  }
  else
  {
    // replace first_nd by nullptr in the tree_occs of first_edge
    int k;
    for(k=0; first_nd != dc->Gp->tree_occ[first_edge][i][k]; k++);
    dc->Gp->tree_occ[first_edge][i][k] = nullptr;
  }

  // --- update edge_occs --- //
  // right edge of first_nd becomes right edge of last node
  last_nd->edge_occ[rnb_right] = first_edge;

  // left edge of en becomes left edge of new_occ
  new_occ->edge_occ[rnb_left] = en->edge_occ[rnb_left];
  en->edge_occ[rnb_left] = nullptr;

  // --- update the et_tree --- //
  // split off first occurrence and delete it
  rnb_tree s1, s2;
  split(first_nd,rnb_right,s1,s2,dc->et_dummy);
  delete first_nd;

  // split immediately before en
  split(en,rnb_left,s1,s2,dc->et_dummy);

  // join the pieces
  et = et_join((et_tree)s2,et_join((et_tree)s1,new_occ,dc->et_dummy),
                dc->et_dummy);
}

et_tree et_link(node u, node v, edge e, int i, dyn_con* dc)
// Modify the et_trees at level i corresponding to the insertion of
// the edge e = (u,v) into F_i.
// Prec.: u and v belong to dc, and they are not connected at the
//        valid level i.
{
  // get active occurrences of u and v, create a new occurrence of u
  et_node u_act = dc->Gp->act_occ[u][i];
  et_node v_act = dc->Gp->act_occ[v][i];
  et_node new_u_occ = new et_node_struct(u_act);

  // find the tree etv containing v_act and reroot it at v_act
  et_tree etv = v_act->find_root();
  change_root(etv,v_act,i,dc);

  // --- initialize tree_occs of e ---
  // u_act and new_u_occ become the first two tree_occs of e
  dc->Gp->tree_occ[e][i][0] = u_act;
  dc->Gp->tree_occ[e][i][1] = new_u_occ;

  // the first and the last node of etv are tree_occ[i][2 and 3] if
  // they are different otherwise tree_occ[i][2] is nullptr
  et_node etv_last = etv->last();
  dc->Gp->tree_occ[e][i][3] = etv_last;
  if(etv_last != v_act) dc->Gp->tree_occ[e][i][2] = v_act;
  else                  dc->Gp->tree_occ[e][i][2] = nullptr;

  // --- update tree_occs of the edge following e if it exists ---
  edge after_e = u_act->edge_occ[rnb_right];
  if(after_e)
  {
    if(u_act->edge_occ[rnb_left] != after_e)
    {
      // replace u_act by new_u_occ
      int k;
      if(dc->Gp->tree_occ[after_e] == nullptr) { std::cout << "SHOULD NOT HAPPEN" << std::endl; };
      for(k=0; u_act != dc->Gp->tree_occ[after_e][i][k]; k++);
      dc->Gp->tree_occ[after_e][i][k] = new_u_occ;
    }
    else
    {
      // replace nullptr pointer by new_u_occ
      int k;
      for(k=0; nullptr != dc->Gp->tree_occ[after_e][i][k]; k++);
      dc->Gp->tree_occ[after_e][i][k] = new_u_occ;
    }
  }

  // --- update edge_occs ---
  new_u_occ->edge_occ[rnb_right] = u_act->edge_occ[rnb_right];
  new_u_occ->edge_occ[rnb_left] = e;
  u_act->edge_occ[rnb_right] = e;
  v_act->edge_occ[rnb_left] = e;
  etv_last->edge_occ[rnb_right] = e;

  // --- update et_trees ---
  // concatenate etv and the new occurrence
  etv = et_join(etv,new_u_occ,dc->et_dummy);

  // split the et_tree containing u_act after u_act
  rnb_tree s1, s2;
  split(u_act,rnb_right,s1,s2,dc->et_dummy);

  // concatenate the pieces
  return et_join((et_tree)s1,et_join(etv,(et_tree)s2,dc->et_dummy),
                  dc->et_dummy);
}

void et_cut(edge e, int i, dyn_con* dc)
// Update the et_trees at level corresponding to the removal of
// the tree edge e.
// Prec.: e actually is a tree edge at level i.
{
  // get the et_nodes representing e at level i
  et_node ea1 = dc->Gp->tree_occ[e][i][0];
  et_node ea2 = dc->Gp->tree_occ[e][i][1];
  et_node eb1 = dc->Gp->tree_occ[e][i][2];
  et_node eb2 = dc->Gp->tree_occ[e][i][3];

  // set the tree_occ to nullptr;
  dc->Gp->tree_occ[e][i][0] = nullptr;
  dc->Gp->tree_occ[e][i][1] = nullptr;
  dc->Gp->tree_occ[e][i][2] = nullptr;
  dc->Gp->tree_occ[e][i][3] = nullptr;

  // sort ea1, ea2, eb1, and eb2, such that
  // ea1 < eb1 < eb2 < ea2 in In-order if they are not nullptr
  // eb1 may be nullptr
  et_node aux;
  if(ea1 && ea2)
  {
    if(smaller(ea2,ea1))
    {
      aux = ea1; ea1 = ea2; ea2 = aux;
    }
  }
  else      // either ea1 or ea2 is nullptr...
  {
    if(ea1) // ...it is ea2
    {
      ea2 = ea1; ea1 = nullptr;
    }
  }

  if(eb1 && eb2)
  {
    if(smaller(eb2,eb1))
    {
      aux = eb1; eb1 = eb2; eb2 = aux;
    }
  }
  else      // either eb1 or eb2 is nullptr...
  {
    if(eb1) // ...it is eb2
    {
      eb2 = eb1; eb1 = nullptr;
    }
  }

  // now ea2 and eb2 are non-nullptr
  if(smaller(ea2,eb2))
  {
    aux = eb1; eb1 = ea1; ea1 = aux;
    aux = eb2; eb2 = ea2; ea2 = aux;
  }

  // --- update et_trees ---
  // compute s1, s2 and s3
  rnb_tree s1, s2, s3;
  split(ea1,rnb_right,s1,s2,dc->et_dummy);
  split(ea2,rnb_right,s2,s3,dc->et_dummy);

  // compute the first of the two resulting trees
  et_join((et_tree)s1,(et_tree)s3,dc->et_dummy);

  // split off ea2 from s2 giving the second tree
  split(eb2,rnb_right,s1,s2,dc->et_dummy);

  // --- update active occurrences ---
  if(ea2->active) ea2->pass_activity(ea1);

  // --- update tree_occs ---
  // update tree_occs of the edge following e if it exists
  edge after_e = ea2->edge_occ[rnb_right];
  if(after_e)
  {
    if(ea1->edge_occ[rnb_left] != after_e)
    {
      // replace ea2 by ea1
      int k;
      if(dc->Gp->tree_occ[after_e] == nullptr) { std::cout << "SHOULD NOT HAPPEN" << std::endl; };
      for(k=0; ea2 != dc->Gp->tree_occ[after_e][i][k]; k++);
      dc->Gp->tree_occ[after_e][i][k] = ea1;
    }
    else
    {
      // replace ea2 by nullptr
      int k;
      for(k=0; ea2 != dc->Gp->tree_occ[after_e][i][k]; k++);
      dc->Gp->tree_occ[after_e][i][k] = nullptr;
    }
  }

  // --- update edge_occs --- //
  ea1->edge_occ[rnb_right] = ea2->edge_occ[rnb_right];
  if(eb1) eb1->edge_occ[rnb_left] = nullptr;
  else    eb2->edge_occ[rnb_left] = nullptr;
  eb2->edge_occ[rnb_right] = nullptr;

  delete ea2;
}

void et_node_struct::print()
// we redefine print in order to output also the additional fields
{
  // output base fields
  rnbw_node_struct::print();

  // output new fields
  cout << "  dc:          " << dc << "\n";
  cout << "  corr_node:   " << corr_node->index() << "\n";
  cout << "  level:       " << level << "\n";
  cout << "  active:      " << active << "\n";
  if(edge_occ[0])
  {
    cout << "  edge_occ[0]: " << "(" << edge_occ[0]->source()->index();
    cout << "," << edge_occ[0]->target()->index() << ")\n";
  }
  else cout << "  edge_occ[0]: nullptr\n";
  if(edge_occ[1])
  {
    cout << "  edge_occ[1]: " << "(" << edge_occ[1]->source()->index();
    cout << "," << edge_occ[1]->source()->index() << ")\n";
  }
  else cout << "  edge_occ[1]: nullptr\n";
}

