// ------------------------------------------------------------- //
// dyn_con.c: implementation of internal functions and user      //
//            interface functions for the implementation of      //
//            the dynamic connectivity algorithm by              //
//            M. Rauch Henzinger and V. King.                    //
//                                                               //
//            See also the documentation in dyn_con.ps.          //
//                                                               //
// Version 1.3                                                   //
//                                                               //
// Copyright (C) 1995 by David Alberts                           //
// FU Berlin, Inst. f. Informatik, Takustr. 9, 14195 Berlin, FRG //
// All rights reserved.                                          //
// There is ABSOLUTELY NO WARRANTY.                              //
// ------------------------------------------------------------- //

#include "dyn_con.h"
#include <ogdf/basic/Queue.h>
#include <sys/time.h>
#include <limits>

using ogdf::edge;
using ogdf::node;
using ogdf::List;

std::mt19937 dyn_con::random_generator;  // define the static member

edge dyn_con::ins(node u, node v)
// create an edge connecting u and v and return it
{
#ifdef STATISTICS
  n_ins++;
#endif
  // create the new edge
  edge e = Gp->newEdge(u,v);
  Gp->tree_occ[e] = nullptr;

  // test whether u and v are already connected
  if(!connected(u,v,max_level))
  // they are not, so e becomes a forest edge at level max_level
  {
    insert_tree(e,max_level,true);
    added_edges[max_level]++;
    rebuild(max_level);
  }
  else
  // u and v are already connected, find lowest such level
  {
    // current level
    int curr_level = max_level/2;
    // lower bound and upper bound
    int lower = 0;
    int upper = max_level;
    while(curr_level != lower)
    {
      if(connected(u,v,curr_level))
      // search below current level
      {
        upper = curr_level;
        curr_level = (lower + curr_level)/2;
      }
      else
      // search above current level
      {
        lower = curr_level;
        curr_level = (upper + curr_level)/2;
      }
    }
    // Now depending on parity either
    // a) connected(u,v,lower-1) == false && connected(u,v,lower) == true or
    // b) connected(u,v,lower) == false && connected(u,v,lower+1) == true holds.

    // handle case a),
    if(!connected(u,v,lower)) lower++;

    // insert e at appropriate level,
    insert_non_tree(e,lower);
    added_edges[lower]++;
    rebuild(lower);
  }
  return e;
}

void dyn_con::del(edge e)
// delete the edge e
{
#ifdef STATISTICS
  n_del++;
#endif

  // if e is not an edge in F
  if(!tree_edge(e)) {
      delete_non_tree(e);
  } else
  // e is a tree edge
  {
    // remember e
    int e_level = Gp->level[e];
    node u = e->source();
    node v = e->target();

    // remove e
    delete_tree(e);

    // delete specific information for tree edges stored at e
    for(int j=0; j<=max_level; j++) delete[] Gp->tree_occ[e][j];
    delete[] Gp->tree_occ[e];

    // look for a replacement edge
    replace(u,v,e_level);
  }

  // delete information stored at e
  Gp->delEdge(e);
}

bool dyn_con::connected(node u, node v)
// return true if u and v are connected in the current graph
// and false otherwise
{
#ifdef STATISTICS
  n_query++;
#endif

  return connected(u,v,max_level);
}

void dyn_con::print_statistics(std::ostream& out)
{
#ifdef STATISTICS
  out << "\nStatistics\n==========\n";
  out << "number of nodes: " << Gp->number_of_nodes();
  out << " final number of edges: " << Gp->number_of_edges() << "\n\n";
  out << "user supplied operations\n";
  out << "number of ins operations: " << n_ins << "\n";
  out << "number of del operations: " << n_del << "\n";
  out << "number of connected operations: " << n_query << "\n\n";
  out << "internal variables\n";
  out << "number of levels: " << max_level+1 << "\n";
  out << "bound for rebuilds on highest level: " << rebuild_bound[max_level];
  out << "\nsmall_weight: " << small_weight << "\n";
  out << "maximum number of edges to sample: " << edges_to_sample << "\n";
  out << "small_set: " << small_set << "\n\n";
  out << "internal functions\n";
  out << "number of connected operations: " << n_connected << "\n";
  out << "number of insert_tree operations: " << n_ins_tree << "\n";
  out << "number of delete_tree operations: " << n_del_tree << "\n";
  out << "number of replace operations: " << n_replace << "\n";
  out << "  weight of T_1 too small: " << rep_small_weight << "\n";
  out << "  case 2.(b): " << rep_succ << "\n";
  out << "  case 3.(b): " << rep_big_cut << "\n";
  out << "  case 3.(c): " << rep_sparse_cut << "\n";
  out << "  case 3.(d): " << rep_empty_cut << "\n";
  out << "number of sample_and_test operations: " << n_sample_and_test << "\n";
  out << "number of get_cut_edges operations without recursive calls: ";
  out << n_get_cut_edges << "\n";
  out << "number of insert_non_tree operations: " << n_ins_non_tree << "\n";
  out << "number of delete_non_tree operations: " << n_del_non_tree << "\n";
  out << "number of move_edges: " << n_move_edges << "\n";
  out << "number of edges moved up: " << edges_moved_up << "\n";
  out << "number of edges moved down: " << edges_moved_down << "\n";
#else
  out << "\ndyn_con::print_statistics: sorry, no statistics available\n";
  out << "  compile libdc.a with -DSTATISTICS to get statistics\n\n";
#endif
}



bool dyn_con::connected(node x, node y, int i)
// Return true if x and y are connected on level i. Otherwise
// return false.
{
#ifdef STATISTICS
  n_connected++;
#endif

  // get the active occurrences of x and y at level i
  et_node x_act_occ = Gp->act_occ[x][i];
  et_node y_act_occ = Gp->act_occ[y][i];

  // return whether they belong to the same tree at level i
  return (x_act_occ->find_root() == y_act_occ->find_root());
}

void dyn_con::insert_tree(edge e, int i, bool create_tree_occs)
// Insert e into F_i. If create_tree_occs is true, then the storage for the
// tree_occ array for e is allocated.
{
#ifdef STATISTICS
  n_ins_tree++;
#endif

  // find the endpoints of e
  node u = e->source();
  node v = e->target();

#ifdef DEBUG
  std::cout << "(" << u->index() << "," << v->index() << ") tree ins at level ";
  std::cout << i << "\n";
#endif

  // enter level of e
  Gp->level[e] = i;

  // create tree_occ array for e if requested
  if(create_tree_occs)
  {
    Gp->tree_occ[e] = new et_node*[max_level+1];
    for(int lev=0; lev<=max_level; lev++)
    {
      Gp->tree_occ[e][lev] = new et_node[4];
      for(int j=0; j<4; j++) {
        Gp->tree_occ[e][lev][j] = nullptr;
      }
    }
  }

  // link the et_trees containing the active occurrences of u and v
  for(int j=i; j<=max_level; j++) et_link(u,v,e,j,this);

  // append e to the list of tree edges at level i
  Gp->tree_item[e] = tree_edges[i].pushBack(e);
}

void dyn_con::delete_tree(edge e)
// Remove the tree edge e from F.
{
#ifdef STATISTICS
  n_del_tree++;
#endif

  // get the level of e
  int i = level(e);

#ifdef DEBUG
  std::cout << "(" << e->source()->index() << "," << e->target()->index() << ") tree del ";
  std::cout << "at level " << i << "\n";
#endif

  // cut the spanning trees
  for(int j=i; j<=max_level; j++) et_cut(e,j,this);

  // remove e from the list of tree edges at level i
  tree_edges[i].del(Gp->tree_item[e]);

  // set tree_item of e to null
  Gp->tree_item[e] = nullptr;
}

void dyn_con::replace(node u, node v, int i)
// try to reconnect the trees on level i containing u and v.
// if not possible try to recurse on higher level
{
#ifdef STATISTICS
  n_replace++;
#endif

  // determine the level i trees containing u and v
  et_tree t1 = Gp->act_occ[u][i]->find_root();
  et_tree t2 = Gp->act_occ[v][i]->find_root();

  // let t1 be the smaller tree
  if(t1->get_subtree_weight() > t2->get_subtree_weight()) t1 = t2;

  int handle_cut = false;
  if(t1->get_subtree_weight() > small_weight)
  {
    // sample randomly at most edges_to_sample edges
    int not_done = true;
    edge e = nullptr;
    for(int j=0; not_done && (j<edges_to_sample); j++)
    {
      e = sample_and_test(t1,i);
      if(e) not_done = false;
    }

    if(e != nullptr)
    {
      // sampling was successful, insert e as a tree edge at level i
      delete_non_tree(e);
      insert_tree(e,i,true);
#ifdef STATISTICS
      rep_succ++;
#endif
    }
    else // sampling not successful
    {
      handle_cut = true;
    }

  }
  else // weight of T_1 too small (too few adjacent edges)
  {
    handle_cut = true;
#ifdef STATISTICS
    rep_small_weight++;
#endif
  }

  if(handle_cut)
  // sampling was unsuccessful or too few edges
  {
    // determine all edges crossing the cut
    List<edge> cut_edges;
    if(t1->get_subtree_weight() > 0)
    {
      get_cut_edges(t1,i,cut_edges);
#ifdef STATISTICS
      n_get_cut_edges++;
#endif
    }

    if(cut_edges.size() == 0)
    // no replacement edge on this level, recurse on higher level if possible
    {
#ifdef STATISTICS
      rep_empty_cut++;
#endif
      if(i<max_level) replace(u,v,i+1);
    }
    else  // cut_edges.size() > 0
    {
      if(cut_edges.size() >= t1->get_subtree_weight()/small_set)
      // if cut_edges is big enough we reconnect t1 and t2 on level i
      {
#ifdef STATISTICS
        rep_big_cut++;
#endif
        edge reconnect = cut_edges.front();
        cut_edges.popFront();
        delete_non_tree(reconnect);
        insert_tree(reconnect,i,true);
      }
      else
      {
        // 0 < cut_edges.size() < t1->get_subtree_weight()/small_set
        // there are too few edges crossing the cut
#ifdef STATISTICS
        rep_sparse_cut++;
#endif
        edge reconnect = cut_edges.front();
        cut_edges.popFront();
        delete_non_tree(reconnect);

        if(i<max_level)
        {
          // move cut edges one level up
          insert_tree(reconnect,i+1,true);
          added_edges[i+1]++;
          for(edge &e : cut_edges)
          {
            delete_non_tree(e);
            insert_non_tree(e,i+1);
            added_edges[i+1]++;
          }
#ifdef STATISTICS
          edges_moved_up += cut_edges.size() + 1;
#endif
          rebuild(i+1);
        }
        else     // on top level, no moving of edges
        {
          insert_tree(reconnect,i,true);
        }
      }
    }
  }
}

edge dyn_con::sample_and_test(et_tree T, int i)
// Randomly select a non-tree edge of G_i that has at least one
// endpoint in T, where an edge with both endpoints in T is picked
// with 2/w(T) and an edge with exactly one endpoint in T is picked
// with probability 1/w(T).
// Test if exactly one endpoint is in T, and if so return the edge.
// Otherwise return nil.
{
#ifdef STATISTICS
  n_sample_and_test++;
#endif

  // get the number of adjacencies
  int no_of_adj = T->get_subtree_weight();

  // pick a random one
  std::uniform_int_distribution<std::mt19937::result_type> uniform(1, no_of_adj);
  int rnd_adj = uniform(random_generator);;

  // locate the et_node representing this adjacency and get the corr. node
  int offset;
  et_node et_repr = et_locate(T,rnd_adj,offset);
  node u = et_repr->get_corr_node();

  // locate the edge corresp. to offset adjacent to u at level i
  ed_node en = ed_locate(Gp->adj_edges[u][i],offset,offset);
  edge e = en->get_corr_edge();

  // get the second node of e
  node v = (e->source() == u) ? e->target() : e->source();

  // if v is in a different tree at level i then return e else nil
  if(connected(u,v,i)) return nullptr;
  else                 return e;
}

void dyn_con::traverse_edges(ed_node ed, List<edge>& edge_list)
// append edges with exactly one endpoint in subtree rooted at ed to edge_list
// auxiliary function called by get_cut_edges
{
  if(ed)
  {
    edge e = ed->get_corr_edge();
    if(!connected(e->source(),e->target(),Gp->level[e]))
    {
      // only one endpoint of e in current spanning tree -> append edge
      edge_list.pushBack(e);
    }

    traverse_edges(ed->left_child(),edge_list);
    traverse_edges(ed->right_child(),edge_list);
  }
}

void dyn_con::get_cut_edges(et_node u, int level, List<edge>& result)
// Return the edges with exactly one endpoint in the et_tree rooted at u
// in result.
{
  if(u && u->get_subtree_weight())
  {
    node v = u->get_corr_node();
    if(u->is_active()) traverse_edges(Gp->adj_edges[v][level],result);
    get_cut_edges(u->left_child(),level,result);
    get_cut_edges(u->right_child(),level,result);
  }
}

void dyn_con::insert_non_tree(edge e, int i)
// Insert the non-tree edge e into G_i.
{
#ifdef STATISTICS
  n_ins_non_tree++;
#endif

#ifdef DEBUG
  std::cout << "(" << e->source()->index() << "," << e->target()->index();
  std::cout << ") non-tree ins at level " << i << "\n";
#endif

  Gp->level[e] = i;
  node u = e->source();
  node v = e->target();

  // insert e in the adjacency trees of its endpoints at level i
  Gp->non_tree_occ[e][0] =
                   ed_insert(Gp->adj_edges[u][i],e,ed_dummy);
  Gp->non_tree_occ[e][1] =
                   ed_insert(Gp->adj_edges[v][i],e,ed_dummy);

  // update non_tree_edges[i]
  Gp->non_tree_item[e] = non_tree_edges[i].pushBack(e);

  // increase the weight of the active occurrences of u and v at level i
  Gp->act_occ[u][i]->add_weight(1);
  Gp->act_occ[v][i]->add_weight(1);
}

void dyn_con::delete_non_tree(edge e)
// Delete the non-tree edge e.
{
#ifdef STATISTICS
  n_del_non_tree++;
#endif

  // find the endpoints and the level of e
  node u = e->source();
  node v = e->target();
  int i = Gp->level[e];

#ifdef DEBUG
  std::cout << "(" << e->source()->index() << "," << e->target()->index();
  std::cout << ") non-tree del at level " << i << "\n";
#endif

  // remove e from the ed_trees of u and v at level i
  ed_delete(Gp->adj_edges[u][i],Gp->non_tree_occ[e][0],ed_dummy);
  Gp->non_tree_occ[e][0] = nullptr;
  ed_delete(Gp->adj_edges[v][i],Gp->non_tree_occ[e][1],ed_dummy);
  Gp->non_tree_occ[e][1] = nullptr;

  // remove e from the list of non-tree edges at level i
  non_tree_edges[i].del(Gp->non_tree_item[e]);
  Gp->non_tree_item[e] = nullptr;

  // decrease the weights of the active occurrences of u and v if they exist
  Gp->act_occ[u][i]->add_weight(-1);
  Gp->act_occ[v][i]->add_weight(-1);
}

void dyn_con::rebuild(int i)
// does a rebuild at level i if necessary
{
  // rebuilds take place only at level 3 and higher
  if(i<3) return;

  // count added edges at level j>=i
  int sum_added_edges = 0;
  for(int j=i; j<=max_level; j++) sum_added_edges += added_edges[j];

  if(sum_added_edges > rebuild_bound[i])
  {
#ifdef DEBUG
      std::cout << "rebuild(" << i << ")\n";
#endif
    // move edges down
    move_edges(i);
    for(int j=i; j<=max_level; j++) added_edges[j] = 0;
  }
}

void dyn_con::move_edges(int i)
// For j>=i, insert all edges of F_j into F_{i-1}, and all
// non-tree edges of G_j into G_{i-1}.
{
#ifdef STATISTICS
  n_move_edges++;
#endif

  // for each level starting at max_level and ending at i...
  for(int j=max_level; j>=i; j--)
  {
#ifdef STATISTICS
    edges_moved_down += non_tree_edges[j].size() + tree_edges[j].size();
#endif
    // move non-tree edges
    while(non_tree_edges[j].size())
    {
      edge e = non_tree_edges[j].front();
      // delete non-tree edge at level j ...
      delete_non_tree(e);
      // ... and insert it into level i-1
      insert_non_tree(e,i-1);
    }

    // move tree edges
    while(tree_edges[j].size())
    {
      edge e = tree_edges[j].front();

      // update tree_edges[j], tree_edges[i-1], tree_item and level
      tree_edges[j].del(Gp->tree_item[e]);
      Gp->tree_item[e] = tree_edges[i-1].pushBack(e);
      Gp->level[e] = i-1;

      // link the corresponding et_trees from level i-1 to j-1
      for(int k=i-1; k<j; k++)
      {
        et_link(e->source(),e->target(),e,k,this);
      }
    }
  }
}

dyn_con::dyn_con(DCGraph& G, int ml_reb_bound, int n_levels,
                 int edges_to_samp, int small_w, int small_s)
// constructor, initializes the dynamic connectivity data structure
// if ml_reb_bound >= 1 it specifies rebuild_bound[max_level] (default is 5000)
// if n_levels > 0 then it specifies the number of levels (default O(log n))
// if edges_to_samp >= 0 then it specifies edges_to_sample (default 16 log^2 n)
// if small_w >= 0 then it specifies small_weight (default log^2 n)
// if small_s >= 1 then it specifies small_set (default 8 log n)
{
  // --- initialize the constants ---
  Gp = &G;
  int log_n = 0;
  int i;
  for(i = G.numberOfNodes(); i; i /= 2) log_n++;

  if(small_w>=0) small_weight = small_w;
  else           small_weight = log_n * log_n;

  if(edges_to_samp>=0) edges_to_sample = edges_to_samp;
  else                 edges_to_sample = 16 * log_n * log_n;

  if(small_s>=1) small_set = small_s;
  else           small_set = 8 * log_n;

  if(n_levels > 0) max_level = n_levels - 1;
  else
  {
    max_level = 2 * log_n;
    for(int k=4; (k<ml_reb_bound) && (max_level>=2); k *= 2, max_level--);
  }

#ifdef DEBUG
  std::cout << "|V(G)|          = " << G.numberOfNodes() << "\n";
  std::cout << "max_level       = " << max_level << "\n";
  std::cout << "edges_to_sample = " << edges_to_sample << "\n";
  std::cout << "small_set       = " << small_set << "\n\n";
#endif

  // --- initialize dummy nodes ---
  et_dummy = new et_node_struct(this,nullptr);
  ed_dummy = new ed_node_struct(nullptr);

  // --- initialize the edge lists ---
  non_tree_edges = new List<edge> [max_level+1];
  tree_edges = new List<edge> [max_level+1];

  // --- initialize added_edges ---
  added_edges = new int[max_level+1];
  for(i=0; i<=max_level; i++) added_edges[i] = 0;

  // --- initialize rebuild_bound ---
  rebuild_bound = new int[max_level+1];
  int bound;
  if (ml_reb_bound>=1) bound = ml_reb_bound;
  else                 bound = 5000;
  for (int k=max_level; k>=0; k--)
  {
    rebuild_bound[k] = bound;
    if (bound < std::numeric_limits<int>::max()/2) {
        bound *= 2;  // double the bound if possible
    }
  }

  // --- initialize the nodes ---
  for (node u : G.nodes)
  {
    G.act_occ[u] = new et_node[max_level+1];
    G.adj_edges[u] = new ed_tree[max_level+1];
    for(i=0; i<=max_level; i++)
    {
      G.act_occ[u][i] = new et_node_struct(this,u,i,true);
      G.adj_edges[u][i] = nullptr;
    }
  }

#ifdef STATISTICS
  // --- initialize variables touched by initializing edges ---
  n_connected = 0;
  n_ins_tree = 0;
  n_ins_non_tree = 0;
#endif

  // --- initialize the edges ---
  for (edge e : G.edges)
  {
    Gp->tree_occ[e] = nullptr;
    if(!connected(e->source(),e->target(),0)) {
        insert_tree(e,0,true);
    } else {
        insert_non_tree(e,0);
    }
  }

#ifdef STATISTICS
  // --- initialize statistics ---
  n_ins = 0;
  n_del = 0;
  n_query = 0;
  n_connected = 0;
  n_ins_tree = 0;
  n_del_tree = 0;
  n_replace = 0;
  rep_succ = 0;
  rep_big_cut = 0;
  rep_sparse_cut = 0;
  rep_empty_cut = 0;
  rep_small_weight = 0;
  n_sample_and_test = 0;
  n_get_cut_edges = 0;
  n_ins_non_tree = 0;
  n_del_non_tree = 0;
  n_move_edges = 0;
  edges_moved_up = 0;
  edges_moved_down = 0;
#endif
}

dyn_con::~dyn_con()
{
  // first delete all edges in the data structure (not in G)
  for (edge e : Gp->edges)
  {
    if(tree_edge(e))
    {
      delete_tree(e);
      for(int j=0; j<=max_level; j++) {
          delete[] Gp->tree_occ[e][j];
      }
      delete[] Gp->tree_occ[e];
    }
    else delete_non_tree(e);
  }

  // delete fields (edge lists are empty, no need to clear() them)
  delete[] non_tree_edges;
  delete[] tree_edges;
  delete[] added_edges;
  delete[] rebuild_bound;

  // delete the et_nodes and the information at the nodes of G
  for (node v : Gp->nodes)
  {
    // per node of G only its active occurrence at each level is left
    for(int i=0; i<=max_level; i++) {
        delete Gp->act_occ[v][i];
    }

    delete[] Gp->act_occ[v];
    delete[] Gp->adj_edges[v];
  }

  // delete dummy nodes
  delete et_dummy;
  delete ed_dummy;
}
