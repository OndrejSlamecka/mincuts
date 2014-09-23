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

#include <iostream>
#include <stdexcept>
#include <ogdf/basic/Queue.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Stack.h>
#include <ogdf/basic/simple_graph_alg.h>

#include "helpers.hpp"
#include "coloredgraph.h"

using namespace std;
using namespace ogdf;

int m; // Cocircuit size bound

/**
 * @brief Takes a csv file with lines "<id>;<source>;<target>;<edge name>;..." and transforms it into graph
 * @param sEdgesFileName
 * @return Graph
 */
ColoredGraph csvToGraph(string sEdgesFileName) { // This copies Graph on exit, to speed up, extend Graph and let this be a new method in our extended class
    ColoredGraph G;

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
List<edge> shortestPath(const ColoredGraph &G, node s, node t, List<edge> forbidden) {
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
            if (forbidden.search(e).valid() || G[e] == Color::RED) continue; // TODO: Use BST or array (id -> bool) to represent forbidden?

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
 * Explores the component from node s and counts nodes colored blue, stops if nBluesInGraph is reached
 */
int countConnectedBlueNodes(const ColoredGraph &G, const node &s, int nBluesInGraph) {
    int nFoundBlues = 0;

    NodeArray<bool> discovered(G, false);
    Stack<node> S;
    S.push(s);

    node v, u;
    adjEntry adj;
    while(!S.empty()) {
        v = S.pop();

        if (!discovered[v]) {
            discovered[v] = true;

            if (G[v] == Color::BLUE) {
                nFoundBlues++;
                if (nFoundBlues == nBluesInGraph) break; // There can't be more blue vertices
            }

            forall_adj(adj, v) {
                u = adj->twinNode();
                S.push(u);
            }
        }
    }

    return nFoundBlues;
}

/**
 * E(G)\X contains a k-way hyperplane of G\X iff G\V(R) contains a connected subgraph G_b spanning all the blue vertices of X
 * @param G
 * @param X
 * @param coloring
 * @param blue
 * @return
 */
bool hasHyperplane(ColoredGraph &G, List<edge> X) {
    for(List<edge>::iterator iterator = X.begin(); iterator != X.end(); ++iterator) {
        G.hideEdge(*iterator);
    }

    node v, s;
    int nBlues = 0;

    forall_nodes(v, G) {
        if (G[v] == Color::BLUE) {
            nBlues++;
            s = v;
        }
    }

    int nBluesReached = 0;

    if (nBlues == 0) { // TODO: THIS SHOULD NEVER HAPPEN (yet it does... how to fix? Never color red any blue node in X)
        G.restoreAllEdges();
        //cout << "ups it happened" << endl;
        return false;
    } else {
        // Perform a DFS on G\X from s and see if all vertices are contained
        nBluesReached = countConnectedBlueNodes(G, s, nBlues);
    }

    G.restoreAllEdges();

    return nBluesReached == nBlues;
}

void GenCocircuits(List<List<edge>> &Cocircuits, ColoredGraph &G, List<edge> X, node red, node blue) {
    if(!hasHyperplane(G, X) || X.size() > m) { // E\X contains no hyperplane of M
        return;
    }

    NodeArray<bool> vX(G, false);
    ListConstIterator<List<edge> > it;
    forall_listiterators(edge, it, X) {
        edge e = *it;
        node u = e->source(), v = e->target();
        vX[u] = true;
        vX[v] = true;
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

                /*if ((G[e->source()] == Color::BLUE && vX[e->source()])
                 || (G[e->target()] == Color::BLUE && vX[e->target()])) {
                    cout << "----" << endl << X << " : " << D << endl;
                    printColoring(G); cout << endl;
                    cout << "recoloring " << e << " and " << (iterator == D.end()) << endl;
                }*/

                G[e->source()] = Color::RED;
                G[e->target()] = Color::RED;
            }

            // Color the rest of the path blue
            for(List<edge>::iterator j = iterator; j != D.end(); j++) {
                edge e = *j;
                if (G[e->source()] != Color::RED)
                    G[e->source()] = Color::BLUE;

                if (G[e->target()] != Color::RED)
                    G[e->target()] = Color::BLUE;
            }

            GenCocircuits(Cocircuits, G, newX, u, v);
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
        if (!isBackedge[e]) {
            spanningEdges.pushBack(e);
        }
    }

    return spanningEdges;
}

int main(int argc, char* argv[])
{
    if (argc != 3 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        cout << "Usage: " << argv[0] << " <graph.csv> <cocircuit size bound>" << endl;
        exit(1);
    }

    string graphFile(argv[1]);
    m = stoi(argv[2]);
    if (m < 1) {
        cerr << "Cocircuit size bound lower than 1. Terminating." << endl;
        exit(2);
    }

    try {
        ColoredGraph G = csvToGraph(graphFile);
        List<edge> base = spanningEdges(G);

        edge e;
        for(List<edge>::iterator i = base.begin(); i != base.end(); i++) {
            List<List<edge> > Cocircuits;

            e = *i;
            List<edge> X; // (Indexes might be sufficient? Check later)
            X.pushBack(e);
            G[e->source()] = Color::RED;
            G[e->target()] = Color::BLUE;

            GenCocircuits(Cocircuits, G, X, e->source(), e->target());

            for(List<List<edge> >::iterator it = Cocircuits.begin(); it != Cocircuits.end(); ++it) {
                cout << "Cocircuit: " << *it << endl;
            }
        }


    } catch (invalid_argument *e) {
        cerr << "Error: " << e->what() << endl;
        return 1;
    }

    return 0;
}

