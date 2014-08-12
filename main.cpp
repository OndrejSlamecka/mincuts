/**
 *
 * Terminology (Matroid - Graph) for escalated version of algorithm:
 *  Basis - Spanning forest
 *  Independent set - Tree (i.e. subset of a basis)
 *  Circuit - cycle
 *  Cocircuit - minimal edge-cut
 *  Hyperplane - maximal set not containing any basis (= complement of a min-cut)
 *
 *  Spanning forest - union of spanning trees of each component of an unconnected graph
 *
 */

enum {
    BLACK,
    RED,
    BLUE
};

#include "helpers.hpp"

#include <iostream>
#include <stdexcept>
#include <ogdf/basic/Queue.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Stack.h>
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

/**
 * E(G)\X contains a k-way hyperplane of G\X iff G\V(R) contains a connected subgraph G_b spanning all the blue vertices of X
 * @param G
 * @param X
 * @param coloring
 * @param blue
 * @return
 */
bool hasHyperplane(Graph &G, List<edge> X, NodeArray<int> &coloring) {
    for(List<edge>::iterator iterator = X.begin(); iterator != X.end(); ++iterator) {
        G.hideEdge(*iterator);
    }

    // Make a list of all blue vertices of X
    List<node> blues;

    for(List<edge>::iterator iterator = X.begin(); iterator != X.end(); ++iterator) {
        edge e = *iterator;
        node u = e->source(), v = e->target();

        if (coloring[u] == BLUE)
            blues.pushBack(u);

        if (coloring[v] == BLUE)
            blues.pushBack(v);
    }

    // Pick one, call it s
    node s = blues.front();

    // Perform a DFS on G\X from s and see if all vertices are contained
    NodeArray<bool> discovered(G, false);
    Stack<node> S;
    S.push(s);

    node v, u;
    adjEntry adj;
    while(!S.empty()) {
        v = S.pop();

        if (!discovered[v]) {
            discovered[v] = true;

            if (coloring[v] == BLUE) { // TODO: If blues were exactly those blues in X, all we would need would be simple count! (Currently the rest of the path D is painted blue)
                blues.removeFirst(v);
                if (blues.empty()) {
                    break; // Don't forget G.restoreAllEdges();!!
                }
            }

            forall_adj(adj, v) {
                u = adj->twinNode();
                S.push(u);
            }
        }
    }

    G.restoreAllEdges();

    return blues.empty();
}

void GenCocircuits(List<List<edge>> &Cocircuits, Graph &G, List<edge> X, NodeArray<int> coloring, node red, node blue) {
    if(!hasHyperplane(G, X, coloring) || X.size() > m) { // E\X contains no hyperplane of M
        return;
    }

    // Find set D = (a short circuit C in G, s. t. |C ∩ X| = 1) \ X
    List<edge> D = shortestPath(G, red, blue, X);
    if (D.size() > 0) {

        // for each c ∈ D, recursively call GenCocircuits(X ∪ {c}).
        for(List<edge>::iterator iterator = D.begin(); iterator != D.end(); iterator++) {            
            edge c = *iterator;            
            List<edge> newX = X;
            newX.pushBack(c);

            node u = c->source(), v = c->target(); // c = (u,v)            

            // Color vertices of c = (u,v)
            // the one closer to any RED vertex, say u, will be red (and so will be all vertices on the path from the first red to u)
            for(List<edge>::iterator j = D.begin(); j != iterator; j++) {
                edge e = *j;
                coloring[e->source()] = RED;
                coloring[e->target()] = RED;
            }

            // Color the rest of the path blue
            for(List<edge>::iterator j = iterator; j != D.end(); j++) {
                edge e = *j;
                if (coloring[e->source()] != RED)
                    coloring[e->source()] = BLUE;

                if (coloring[e->target()] != RED)
                    coloring[e->target()] = BLUE;
            }

            GenCocircuits(Cocircuits, G, newX, coloring, u, v);
        }


    } else {
        // If there is no such circuit C above (line 4), then return ‘Cocircuit: X’.
        Cocircuits.pushBack(X);
    }
}

/**
 * Returns edges spanning forest of (possibly disconnected) graph G
 * @param G
 */
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
        List<List<edge> > Cocircuits;
        Graph G = csvToGraph("data/graph5.csv");

        List<edge> base = spanningEdges(G);
        cout << base << endl;

        NodeArray<int> coloring(G, BLACK);

        edge e;
        for(List<edge>::iterator i = base.begin(); i != base.end(); i++) {
            e = *i;

            List<edge> X; // (Indexes might be sufficient? Check later)
            X.pushBack(e);
            coloring[e->source()] = RED;
            coloring[e->target()] = BLUE;
            GenCocircuits(Cocircuits, G, X, coloring, e->source(), e->target());

            for(List<List<edge> >::iterator it = Cocircuits.begin(); it != Cocircuits.end(); ++it) {
                cout << "Found a cocircuit: " << *it << endl;
            }
        }



    } catch (invalid_argument *e) {
        cerr << "Error: " << e->what() << endl;
        return 1;
    }

    return 0;
}

