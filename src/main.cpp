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
#include "graphcoloring.h"

using namespace std;
using namespace ogdf;

int m; // Cocircuit size bound

/**
 * @brief Takes a csv file with lines "<id>;<source>;<target>;<edge name>;..." and transforms it into graph
 * @param sEdgesFileName 
 */
void csvToGraph(string sEdgesFileName, Graph &G) {
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

        // Skip if there already is an edge between these two nodes
        if (nodes.at(u) && nodes.at(v) &&
            (G.searchEdge(nodes.at(u), nodes.at(v)) || G.searchEdge(nodes.at(v), nodes.at(u))))
            continue;

        if(nodes.at(u) == nullptr)
            nodes[u] = G.newNode(u);

        if(nodes[v] == nullptr)
            nodes[v] = G.newNode(v);

        G.newEdge(nodes[u], nodes[v], id);
    }
}

/**
 * Performs BFS to find the shortest path from s to t in graph g without using any red edges and any edge from the forbidden edges.
 * Returns empty set if no such path exists.
 */
List<edge> shortestPath(const Graph &G, const GraphColoring &coloring, node s, node t, const List<edge> &forbidden) {
    List<edge> path;
    node v, u;
    edge e;

    Queue<node> Q;

    NodeArray<bool> visited(G, false);
    NodeArray<node> predecessor(G);

    Q.append(s);
    visited[s] = true;

    while(!Q.empty()) {
        v = Q.pop();

        if (v == t) {
            // traceback predecessors and reconstruct path
            for (node n = t; n != s; n = predecessor[n]) {
                e = G.searchEdge(n, predecessor[n]); // Takes O(min(deg(v), deg(w))) (that's fast on sparse graphs)
                if (coloring[e] != Color::RED)
                    path.pushFront(e);
            }
            break;
        }

        forall_adj_edges(e, v) {
            // TODO: Use BST or array (id -> bool) to represent forbidden and fasten the following search? (Probably not
            // necessary as forbidden size is bound by the constant m)
            if (forbidden.search(e).valid()) continue;

            u = e->opposite(v);
            if (!visited[u]) {
                predecessor[u] = v;
                visited[u] = true;
                Q.append(u);
            }
        }
    }

    return path;
}

/**
 * @brief hideConnectedBlueSubgraph
 * @param G
 * @param coloring
 * @param u
 */
void hideConnectedBlueSubgraph(Graph &G, const GraphColoring &coloring, node start) {
    Queue<node> Q;
    Q.append(start);

    NodeArray<bool> visited(G, false);

    node u, v;
    edge e;
    while(!Q.empty()) {
        u = Q.pop();
        forall_adj_edges(e, u) {
            if (coloring[e] == Color::BLUE) {
                v = e->opposite(u);
                if (!visited[v]) {
                    visited[v] = true;
                    Q.append(v);
                }
                G.hideEdge(e);
            }
        }
    }
}

bool findPathToAnyBlueAndColorItBlue(const Graph &G, GraphColoring &coloring, node start) {
    Queue<node> Q;
    Q.append(start);

    NodeArray<bool> visited(G, false);
    NodeArray<edge> accessEdge(G);

    node u, n;
    edge e;
    while(!Q.empty()) {
        u = Q.pop();

        if (coloring[u] == Color::BLUE && u != start) {
            // reconstruct path and go on
            edge ae;
            node n1, n2;
            for (n1 = u; n1 != start;) {
                // TODO: Use search edge instead?
                ae = accessEdge[n1];
                n2 = ae->opposite(n1);

                coloring[n1] = Color::BLUE;
                coloring[n2] = Color::BLUE;
                coloring[ae] = Color::BLUE;
                n1 = n2;
            }
            return true;
        }

        forall_adj_edges(e, u) {
            n = e->opposite(u);
            if (!visited[n]) {
                visited[n] = true;
                accessEdge[n] = e;
                Q.append(n);
            }
        }
    }

    return false;
}

void GenCocircuits(List<List<edge>> &Cocircuits, Graph &G, GraphColoring coloring, List<edge> X, node red, node blue) {
    if (X.size() > m) return;

    // Find set D = (a short circuit C in G, s. t. |C ∩ X| = 1) \ X
    List<edge> D = shortestPath(G, coloring, red, blue, X);
    //cout << "now in x: " << X << endl;
    //cout << "r(" << red->index() << ")-b(" << blue->index() << ") path: " << D << endl;
    //printColoring(G, coloring); cout << endl;
    if (D.size() > 0) {

        // for each c ∈ D, recursively call GenCocircuits(X ∪ {c}).
        for(List<edge>::iterator iterator = D.begin(); iterator != D.end(); iterator++) {
            edge c = *iterator;

            //cout << "PROcessing " << c->index() << "(" << c->source()->index() << "," << c->target()->index() << ")" << endl;
            //printColoring(G, coloring);

            List<edge> newX = X;
            newX.pushBack(c);


            // Coloring red-c path red, determining u and v, coloring the rest blue
            node n1 = red, n2 = blue, // after the first of the following for cycles these are the
                                      // nodes of the last red edge (one of them has to be incident with c)
                                      // initialized to red and blue since the first for can have 0 iterations
                 u, v; // c = (u,v), say u is red (and so will be all vertices on the path from the first red to u)

            for(List<edge>::iterator j = D.begin(); j != iterator; j++) {
                edge e = *j;

                n1 = e->source();
                n2 = e->target();

                coloring[n1] = Color::RED;
                coloring[n2] = Color::RED;

                coloring[e] = Color::RED;
            }

            // Determine u, v, s.t. u is really red and v blue
            if (c->source() == n1 || c->target() == n1) { // if n1 is in c then n1 == u
                u = n1;
            } else { // n2 is in c so n2 == u
                u = n2;
            }

            v = c->opposite(u);

            // Color the rest of the path blue
            for(List<edge>::iterator j = iterator.succ(); j != D.end(); j++) {
                edge e = *j;

                coloring[e->source()] = Color::BLUE;
                coloring[e->target()] = Color::BLUE;

                coloring[e] = Color::BLUE;
            }


            // If c = (u, v) is blue, reconnect blue subgraph (find the shortest path from u to v using only nonred edges)

            // if c = (u,v) is blue
            //    if u has blue adjacent edges (except of c) AND v has blue adjacent edges (exc. of c) then
            //       enumerate one blue subgraph, call it G_b1
            //       create graph G_rest = G \ X \ G_r \ G_b1 and BFS in it to until blue is found (use hideEdge!)
            //       (or BFS (avoid red and X) from u to first blue edge not in G_b1)
            //       -> path found, color it blue and continue
            //       -> path not found, fail here (return;)

            if (coloring[c] == Color::BLUE) {
                G.hideEdge(c); // Don't consider c

                bool uaIsEmpty = true, vaIsEmpty = true;
                edge e;

                forall_adj_edges(e, u) {
                    if (coloring[e] == Color::BLUE) {
                        uaIsEmpty = false;
                        break;
                    }
                }

                forall_adj_edges(e, v) {
                    if (coloring[e] == Color::BLUE) {
                        vaIsEmpty = false;
                        break;
                    }
                }

                if (!uaIsEmpty && !vaIsEmpty) {
                    //cout << "X: " << X << "; D: " << D << "; c = " << c->index() << " --- ";
                    //printColoring(G, coloring);
                    //cout<< endl;

                    // G_b has been disconnected, hide X, G_r and one component of blue subgraph (TODO: Maintain G_rest through the whole algorithm and not recompute here every time?)
                    for(List<edge>::iterator it = X.begin(); it != X.end(); it++) G.hideEdge(*it);
                    forall_edges(e, G) { if(coloring[e] == Color::RED) G.hideEdge(e); }
                    hideConnectedBlueSubgraph(G, coloring, u);

                    // BFS in G from u to the first found blue edge
                    //  -> not found => fail here
                    //  -> path found => color it blue and continue

                    if (!findPathToAnyBlueAndColorItBlue(G, coloring, u)) {
                        G.restoreAllEdges();
                        return;
                    }


                } // else c is just a branch with a single leaf, nothing happened G_b stays connected

                G.restoreAllEdges();
            }

            GenCocircuits(Cocircuits, G, coloring, newX, u, v);
        }

    } else {
        // If there is no such circuit C above (line 4), then return ‘Cocircuit: X’.
        //cout << "Cocircuit: " << X << endl;
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

void CircuitCocircuit(Graph &G, List<List<edge>> &cocircuits) {
    List<edge> base = spanningEdges(G);

    edge e;
    for(List<edge>::iterator i = base.begin(); i != base.end(); i++) {
        e = *i;
        List<edge> X; // (Indexes might be sufficient? Check later)
        GraphColoring coloring(G);
        X.pushBack(e);
        coloring[e->source()] = Color::RED;
        coloring[e->target()] = Color::BLUE;

        //cout << "STARTing with edge " << e->index() << " (vertex " << e->source()->index() << " is red)" << endl;

        GenCocircuits(cocircuits, G, coloring, X, e->source(), e->target());
    }
}


/**
 * Helper function todetermine whether given set of edges really is a cut
 */
bool isCut(Graph &G, const List<edge> &cut) {
    // run BFS in G\cut and count found vertices, return |G| != |vertices in BFS tree of G\cut|
}

/**
 * Helper function todetermine whether given set of edges really is a minimal cut
 */
bool isMinCut(Graph &G, const List<edge> &cut) {
    // try to subtract each edge from the cut and test the rest with isCut
}

// TODO: Read from stdin?
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
        Graph G;
        csvToGraph(graphFile, G);

        /*
        List<List<edge>> cocircuits;
        CircuitCocircuit(G, cocircuits);
        for(List<List<edge> >::iterator it = cocircuits.begin(); it != cocircuits.end(); ++it) {
            cout << *it << endl;
        }*/


    } catch (invalid_argument *e) {
        cerr << "Error: " << e->what() << endl;
        return 1;
    }

    return 0;
}

