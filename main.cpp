/**
 *
 * Terminology (Matroid - Graph):
 *  Basis - Spanning tree
 *  Independent set - Tree (i.e. subset of a basis)
 *  Circuit - cycle
 *  Cocircuit - minimal edge-cut
 *  Hyperplane - maximal set not containing any basis
 *
 */

#include "maybe.hpp"
#include "helpers.hpp"

#include <iostream>
#include <stdexcept>
#include <ogdf/basic/Queue.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>

using namespace std;
using namespace ogdf;

int m = 3; // Cocircuit size bound

/**
 * @brief Takes a csv file with lines "<id>;<source>;<target>;<edge name>;..." and transforms it into graph
 * @param sEdgesFileName
 * @return Graph
 */
Graph csvToGraph(string sEdgesFileName) { // This copies Graph on exit, to speed up, extend Graph and let this be a new method in our extended class
    Graph G;

    ifstream fEdges(sEdgesFileName);
    if (!fEdges.is_open())
        throw new invalid_argument("Edges file doesn't exist or could not be accessed");

    vector<node> nodes;
    int id, u, v, nSize = 0;

    for (string line; getline(fEdges, line);) {
        sscanf(line.c_str(), "%d;%d;%d;", &id, &u, &v);

        if (u > nSize) {
            nodes.resize(u + 1);
            nSize = u;
        }

        if (v > nSize) {
            nodes.resize(v + 1);
            nSize = v;
        }

        if(nodes.at(u) == nullptr)
            nodes[u] = G.newNode(u);

        if(nodes[v] == nullptr)
            nodes[v] = G.newNode(v);

        G.newEdge(nodes[u], nodes[v], id);        
    }

    return G;
}

/**
 * Performs BFS to find the shortest path from s to t in graph g without using any edge from the forbidden edges.
 * Returns empty set if no such path exists.
 */
List<edge> shortestPath(const Graph &G, node s, node t, List<edge> forbidden) {
    List<edge> path;
    node v, u;
    edge e;

    Queue<node> Q;

    NodeArray<node> predecessor(G);
    NodeArray<bool> visited(G, false);

    Q.append(s);
    visited[s] = true;

    while(!Q.empty()) {
        v = Q.pop();

        if (v == t) {
            // traceback predecessors and reconstruct path
            for (node n = t; n != s; n = predecessor[n]) {                
                path.pushFront(G.searchEdge(predecessor[n], n));
            }
            break;
        }

        forall_adj_edges(e, v) {
            if (forbidden.search(e).valid()) continue; // TODO: Use BST or array (id -> bool) to represent forbidden?

            u = e->opposite(v);
            if (!visited[u]) {
                predecessor[u] = v; // TODO: Unite predecessor and visited?
                visited[u] = true;
                Q.append(u);
            }
        }
    }    

    return path;
}

int distanceToRed(List<edge> D, node v) {
    int d = 0;

    cout << D << endl;
    exit(1);

    for(;;) {

    }

    return d;
}

Maybe<List<edge> > GenCocircuits(const Graph &G, List<edge> X, NodeArray<int> coloring, node red, node blue) {
    // if(E\X contains no hyperplane of M || X.size() > m)
     //   return return_<Nothing>;

    // Find set D = (a short circuit C in G, s. t. |C ∩ X| = 1) \ X    
    List<edge> D = shortestPath(G, red, blue, X);
    cout << X << D << endl;
    exit(1);
    if (D.size() > 0) {

        // for each c ∈ D, recursively call GenCocircuits(X ∪ {c}).
        for(List<edge>::iterator i = D.begin(); i != D.end(); i++) {
            List<edge> newX = X;
            newX.pushBack(*i); // add c

            coloring[(*i)->source()] = 0;

            // Color vertices of c = (u,v)
            // the one closer to any RED vertex, say u, will be red (and so will be all vertices on the path from the first red to u)

            // 


            return GenCocircuits(G, newX, coloring, red, blue);
        }
    } else {
        // If there is no such circuit C above (line 4), then return ‘Cocircuit: X’.
        return return_<Maybe>(X);
    }    
}

List<edge> spanningEdges(const Graph &G) {
    EdgeArray<bool> isBackedge(G, false);

    List<edge> backedges;
    isAcyclicUndirected(G, backedges);

    for(List<edge>::iterator i = backedges.begin(); i != backedges.end(); i++) {
        isBackedge[*i] = true;
    }

    List<edge> spanningEdges;
    edge e;
    forall_edges(e, G) {
        if (!isBackedge[e])
            spanningEdges.pushBack(e);
    }

    return spanningEdges;
}

int main()
{
    try {
        Graph G = csvToGraph("data/spanning_tree.csv");

        List<edge> base = spanningEdges(G);

        NodeArray<int> coloring(G, 3);

        edge e;
        for(List<edge>::iterator i = base.begin(); i != base.end(); i++) {
            e = *i;

            List<edge> X; // (Indexes might be sufficient? Check later)
            X.pushBack(e);
            coloring[e->source()] = 0;
            coloring[e->target()] = 1;
            Maybe<List<edge> > cocircuit = GenCocircuits(G, X, coloring, e->source(), e->target());
            if(cocircuit.isJust()) {
                cout << "Found a cocircuit: " << cocircuit.fromJust() << endl;
            }
        }



    } catch (invalid_argument *e) {
        cerr << "Error: " << e->what() << endl;
        return 1;
    }

    return 0;
}

