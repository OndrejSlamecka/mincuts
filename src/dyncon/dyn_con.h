#ifndef DCGRAPH
#define DCGRAPH

#include <iostream>
#include <ogdf/basic/List.h>
#include <ogdf/basic/Graph.h>
#include <random>
#include <array>
#include "ed_tree.h"

class dyn_con;
class DCGraph;

class   et_node_struct;
typedef et_node_struct* et_node;
typedef et_node et_tree;

class et_node_struct : public rnbw_node_struct {

public:

  // Constructors
  et_node_struct(dyn_con* dcp, ogdf::node v, int my_level = -1, int activate = false);
  // Create a new et_node_struct at level my_level for v.
  // Activate it if activate is true.

  et_node_struct(et_node en);
  // Create a new et_node_struct which is an inactive copy of en.

  // --- Conversion Functions ---
  et_node parent() { return (et_node) rnbw_node_struct::parent(); }
  et_node left_child() { return (et_node) child[rnb_left]; }
  et_node right_child() { return (et_node) child[rnb_right]; }
  et_tree find_root() { return (et_tree) rnb_node_struct::find_root(); }
  et_node first() { return (et_node) rnb_node_struct::first(); }
  et_node last() { return (et_node) rnb_node_struct::last(); }
  et_node cyclic_succ() { return (et_node) rnb_node_struct::cyclic_succ(); }
  et_node cyclic_pred() { return (et_node) rnb_node_struct::cyclic_pred(); }
  friend inline et_tree et_join(et_tree t1, et_tree t2, et_node dummy)
  { return (et_tree) rnb_join(t1,t2,dummy); }
  friend inline et_node et_locate(et_tree et, int w, int& offset)
  { return (et_node) rnbw_locate(et,w,offset); }

  ogdf::node get_corr_node() { return corr_node; }
  // This et_node is an occurrence of the returned graph node.

  int is_active() { return active; }
  // true <=> active occ.

  friend et_tree et_link(ogdf::node u, ogdf::node v, ogdf::edge e, int i, dyn_con* dc);
  // Modify the et_trees of dc at level i corresponding to the insertion of
  // the edge (u,v) into F_i.
  // Prec.: u and v belong to dc, and they are not connected at the
  //        valid level i.

  friend void et_cut(ogdf::edge e, int i, dyn_con* dc);
  // Update the et_trees at level i corresponding to the removal of
  // the tree edge e.
  // Prec.: e actually is a tree edge at level i.

  void print();
  // Print this node to cout for testing.

protected:

  dyn_con*  dc;          // the dynamic connectivity data structure this node
                         // belongs to
  ogdf::node      corr_node;   // corresponding node in G
  int             level;       // the level of this node
  int             active;      // true iff active occurrence
  ogdf::edge      edge_occ[2]; // the at most two tree edges represented
                         // also by this node. ordered left and right

  void pass_activity(et_node to);
  // Make this node inactive and pass its activity to to.
  // Prec.: This node is active, to represents the same vertex and is on the
  //        same level.

  friend void change_root(et_tree& et, et_node en, int i, dyn_con* dc);
  // Change the root of the tree T represented by the et_tree et to the
  // vertex represented by the et_node en. The new tree is stored at et.
  // Prec.: The et_node en is in the et_tree et.
};


class DCGraph : public ogdf::Graph {
public:
    // node props, comments are for separate nodes
    ogdf::NodeArray<et_node*> act_occ; // array of active occurences
    ogdf::NodeArray<ed_tree*> adj_edges; // array of ed_trees storing adjacent
                                         // non-tree edges

    // edge props, comments are for separate edges
    ogdf::EdgeArray<int> level; // edge belongs to G_level
    ogdf::EdgeArray<ogdf::List<ogdf::edge>::iterator> non_tree_item; // see below
        // non-tree edge: list_item in non_tree_edges[level]
        // tree-edge: null

    ogdf::EdgeArray<ogdf::List<ogdf::edge>::iterator> tree_item; // see below
        // tree edge: list_item in tree_edges[level]
        // non-tree edge: null

    ogdf::EdgeArray<std::array<ed_node, 2>> non_tree_occ; // see below
        // non-tree edge: pointer to the two corresponding ed_nodes
        // tree edge: both are null

    ogdf::EdgeArray<et_node**> tree_occ; // see below
        // tree edge: array for each level the 4 pointers to
        //   occurences repr. this edge
        // non-tree edge: null

    DCGraph() : Graph(),
        act_occ(*this),
        adj_edges(*this),
        level(*this),
        non_tree_item(*this),
        tree_item(*this),
        non_tree_occ(*this),
        tree_occ(*this) {}
};

class dyn_con{

public:

  dyn_con(DCGraph& G, int ml_reb_bound = -1, int n_levels = -1,
          int edges_to_samp = -1, int small_w = -1, int small_s = -1);
  // constructor, initializes the dynamic connectivity data structure
  // if ml_reb_bound >= 1 it specifies rebuild_bound[max_level] (default 5000)
  // if n_levels > 0 then it specifies the number of levels (default O(log n))
  // if edges_to_samp >= 0 then it specifies edges_to_sample
  // (default 16 log^2 n)
  // if small_w >= 0 then it specifies small_weight (default log^2 n)
  // if small_s >= 1 then it specifies small_set (default 8 log n)

  ~dyn_con();
  // destructor

  ogdf::edge ins(ogdf::edge e);
  // create an edge connecting u and v and return it

  void del(ogdf::edge e);
  // delete the edge e

  bool connected(ogdf::node u, ogdf::node v);
  // return true if u and v are connected in the current graph
  // and false otherwise

  void print_statistics(std::ostream& out);
  // prints some statistics to the output stream out

private:

  DCGraph     *Gp;              // pointer to the graph
  int          max_level;       // the maximum level
  ogdf::List<ogdf::edge>  *tree_edges;      // a list of tree edges for each level
  ogdf::List<ogdf::edge>  *non_tree_edges;  // a list of non-tree edges for each level

  int         *added_edges;     // array of #edges added to each level
                                // since last rebuild at lower level
  int         *rebuild_bound;   // array, s.t. sum for j>=i of added_edges[j]
                                // > rebuild_bound[i] <=> rebuild at level i
                                // necessary

  int          small_weight;    // bound used in replace
  int          edges_to_sample; // sample at most this many edges while
                                // searching for a replacement edge for a
                                // deleted tree edge
  int          small_set;       // used in the replacement algorithm, too

  et_node      et_dummy;        // dummy nodes for splitting and joining trees
  ed_node      ed_dummy;

  static thread_local std::mt19937 random_generator;  // for choosing random adjacent edges
                                         // in replace

  // some statistics - these counters are only maintained with
  // the -DSTATISTICS compile option
  int          n_ins;           // number of ins operations
  int          n_del;           // number of del operations
  int          n_query;         // number of user supplied connected queries
  int          n_connected;     // number of conecteds
  int          n_ins_tree;      // number of insert_trees
  int          n_del_tree;      // number of delete_trees
  int          n_replace;       // number of replaces
  int          rep_small_weight;  // w(T_1) small in replace
  int          rep_succ;        // successful sampling
  int          rep_big_cut;     // big cut
  int          rep_sparse_cut;  // sparse cut
  int          rep_empty_cut;   // empty cut
  int          n_sample_and_test; // number of sample_and_test
  int          n_get_cut_edges; // number of invocations of get_cut_edges in
                                // replace (get_cut_edges is recursive)
  int          n_ins_non_tree;  // number of insert_non_tree
  int          n_del_non_tree;  // number of delete_non_tree
  int          n_move_edges;    // number of move_edges
  int          edges_moved_up;  // number of edges moved up (during replace)
  int          edges_moved_down;  // number of edges moved down


  // --- Internal Functions of the dynamic connectivity data structure --- //
  // Let G_i be the subgraph of G on level i, let F_i be the
  // forest in G_i, and let F be the spanning forest of G.
  // Let T be a spanning tree on level i.

  bool connected(ogdf::node x, ogdf::node y, int i);
  // Return true if x and y are connected on level i. Otherwise
  // return false.

  bool tree_edge(ogdf::edge e) {
  // Return true if e is an edge in F, false otherwise.
      return (Gp->tree_occ[e] != nullptr);
  }

  int level(ogdf::edge e)
  // Return i such that e is in G_i.
  { return Gp->level[e]; }

  void insert_tree(ogdf::edge e, int i, bool create_tree_occs = false);
  // Insert e into F_i. If create_tree_occs is true, then the storage for the
  // tree_occ array for e is allocated.

  void delete_tree(ogdf::edge e);
  // Remove the tree edge e from F.

  void replace(ogdf::node u, ogdf::node v, int i);
  // Replace the deleted tree edge (u,v) at level i.

  ogdf::edge sample_and_test(et_tree T, int i);
  // Randomly select a non-tree edge of G_i that has at least one
  // endpoint in T, where an edge with both endpoints in T is picked
  // with 2/w(T) and an edge with exactly one endpoint in T is picked
  // with probability 1/w(T).
  // Test if exactly one endpoint is in T, and if so return the edge.
  // Otherwise return nil.

  void get_cut_edges(et_node et, int i, ogdf::List<ogdf::edge>& cut_edges);
  // Return all non-tree edges with exactly one endpoint in the tree
  // T at level i in cut_edges.

  void traverse_edges(ed_node ed, ogdf::List<ogdf::edge>& cut_edges);
  // Append edges with exactly one endpoint in the subtree rooted at ed
  // to edge_list. This is an auxiliary function called by get_cut_edges.

  void insert_non_tree(ogdf::edge e, int i);
  // Insert the non-tree edge e into G_i.

  void delete_non_tree(ogdf::edge e);
  // Delete the non-tree edge e.

  void rebuild(int i);
  // Rebuild level i if necessary.

  void move_edges(int i);
  // For j>=i, insert all edges of F_j into F_{i-1}, and all
  // non-tree edges of G_j into G_{i-1}.

  friend class et_node_struct;
  friend void change_root(et_tree& et, et_node en, int i, dyn_con* dc);
  friend et_tree et_link(ogdf::node u, ogdf::node v, ogdf::edge e, int i, dyn_con* dc);
  friend void et_cut(ogdf::edge e, int i, dyn_con* dc);
};


#endif
