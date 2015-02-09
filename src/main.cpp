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

// Global variables are evil but these two never change...
int cutSizeBound;
int components;

/**
 * Reads a csv file with lines "<id>;<source>;<target>;..." and transforms it into a graph
 */
void csvToGraph(Graph &G, ifstream &fEdges) {
    string line;

    int id, u, v;
    int maxNodeId = 0;
    for (; getline(fEdges, line);) {
        sscanf(line.c_str(), "%d;%d;%d;", &id, &u, &v);
        if (u > maxNodeId) maxNodeId = u;
        if (v > maxNodeId) maxNodeId = v;
    }

    fEdges.clear(); // This should not be needed in C++11 but as it seems it actually is needed
    fEdges.seekg(0);

    vector<node> nodes(maxNodeId + 1);

    for (; getline(fEdges, line);) {
        sscanf(line.c_str(), "%d;%d;%d;", &id, &u, &v);

        if(nodes[u] == nullptr)
            nodes[u] = G.newNode(u);

        if(nodes[v] == nullptr)
            nodes[v] = G.newNode(v);

        G.newEdge(nodes[u], nodes[v], id);
    }        

}


/**
 * Performs BFS to find the shortest path from s to t in graph G without using any edge from the forbidden edges.
 * Returns empty set if no such path exists.
 */
List<edge> shortestPath(const Graph &G, const GraphColoring &coloring, node s, node t, const List<edge> &forbidden) {
    List<edge> path;
    node u, v;
    edge e;

    Queue<node> Q;

    NodeArray<bool> visited(G, false);
    NodeArray<node> predecessor(G);

    Q.append(s);
    visited[s] = true;

    while(!Q.empty()) {
        v = Q.pop();

        if (v == t) {
            // Traceback predecessors and reconstruct path
            for (node n = t; n != s; n = predecessor[n]) {
                e = G.searchEdge(n, predecessor[n]); // Takes O(min(deg(v), deg(w))) (that's fast on sparse graphs), but TODO: Use access edge vector
                if (coloring[e] != Color::RED)
                    path.pushFront(e);
            }
            break;
        }

        forall_adj_edges(e, v) {
            if (forbidden.search(e).valid()) continue; // This is fast because m is a small constant

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
 * @brief Runs DFS and hides everything which can be reached from start node and is blue
 * @param G
 * @param coloring
 * @param start
 */
void hideConnectedBlueSubgraph(Graph &G, const GraphColoring &coloring, node start) {
    Stack<node> Q;
    Q.push(start);

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
                    Q.push(v);
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

    node u, v;
    edge e;
    while(!Q.empty()) {
        u = Q.pop();

        if (coloring[u] == Color::BLUE && u != start) {
            for (node n = u; n != start; n = v) {
                e = accessEdge[n];
                v = e->opposite(n);

                coloring[n] = Color::BLUE;
                coloring[v] = Color::BLUE;
                coloring[e] = Color::BLUE;
            }
            return true;
        }

        forall_adj_edges(e, u) {
            v = e->opposite(u);
            if (!visited[v]) {
                visited[v] = true;
                accessEdge[v] = e;
                Q.append(v);
            }
        }
    }

    return false;
}


/**
 * @return bool True for successful reconnection, false otherwise
 */
bool reconnectBlueSubgraph(Graph &G, const List<edge> &X, GraphColoring &coloring, node u, node v, edge c) {
    // if u has blue adjacent edges (except of c) AND v has blue adjacent edges (exc. of c) then
    //     enumerate one blue subgraph, call it G_b1
    //     create graph G_rest = G \ X \ G_r \ G_b1 and BFS in it until blue is found
    //     (or BFS (avoid red and X) from u to first blue edge not in G_b1)
    //     -> path found, color it blue and continue
    //     -> path not found, fail here (return;)
    // else c is not a bridge, no disconnection happened, G_b stays connected

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
        // G_b has been disconnected, hide X, G_r and one component of blue subgraph (TODO: Maintain G_rest through the whole algorithm and not recompute here every time?)
        for(List<edge>::const_iterator it = X.begin(); it != X.end(); it++) G.hideEdge(*it);
        forall_edges(e, G) { if(coloring[e] == Color::RED) G.hideEdge(e); }
        hideConnectedBlueSubgraph(G, coloring, u);

        // BFS in G from u to the first found blue edge
        //  -> not found => fail here
        //  -> path found => color it blue and continue

        if (!findPathToAnyBlueAndColorItBlue(G, coloring, u)) {
            G.restoreAllEdges();
            return false;
        }

    }

    G.restoreAllEdges();
    return true;
}


// TODO: pass graph coloring as reference (first colour whole D with blue and then recolor red one edge at a time)
void GenStage(const List<edge> &Y, int j, List<List<edge>> &bonds, Graph &G, GraphColoring coloring, List<edge> X, node red, node blue) {
    if (Y.size() + X.size() > cutSizeBound - components + j + 1) return;

    // Find set P = (a short circuit C in G, s. t. |C ∩ X| = 1) \ X
    List<edge> P = shortestPath(G, coloring, red, blue, X);

    if (P.size() > 0) {

        // for each c ∈ D, recursively call GenCocircuits(X ∪ {c}).
        for(List<edge>::iterator iterator = P.begin(); iterator != P.end(); iterator++) {
            edge c = *iterator;

            List<edge> newX = X;
            newX.pushBack(c);

            // Coloring red-c path red, determining u and v, coloring the rest blue
            node n1 = red, n2 = blue, // after the first of the following for cycles these are the
                                      // nodes of the last red edge (one of them has to be incident with c)
                                      // initialized to red and blue since the first for can have 0 iterations
                 u, v; // c = (u,v), say u is red (and so will be all vertices on the path from the first red to u)

            for(List<edge>::iterator j = P.begin(); j != iterator; j++) {
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
            for(List<edge>::iterator j = iterator.succ(); j != P.end(); j++) {
                edge e = *j;

                coloring[e->source()] = Color::BLUE;
                coloring[e->target()] = Color::BLUE;
                coloring[e] = Color::BLUE;
            }


            // If c = (u, v) is blue, reconnect blue subgraph if needed
            // (if u and v are both blue then find the shortest path between them using only nonred edges)
            if (coloring[c] == Color::BLUE) {
                if(!reconnectBlueSubgraph(G, X, coloring, u, v, c)) { // can't reconnect: fail
                    return;
                }
            }

            GenStage(Y, j, bonds, G, coloring, newX, u, v);
        }

    } else {
        // If there is no such circuit C above (line 6), then return ‘Cocircuit: Y union X’.
        List<edge> un(Y);
        un.conc(X);
        bonds.pushBack(un);
    }
}


// TODO: Define with spanningEdges as parameter passed by reference. Or maybe define own variant of isAcyclicUndirected which wouldn't need that set complement
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

void EscalatedCircuitCocircuit(Graph &G, const List<edge> &Y, int j, List<List<edge>> &bonds) {
    List<edge> D = spanningEdges(G); // D is an arbitrary matroid base; our D corresponds to F from the paper now

    // Set D = E(F) \ Y
    for(List<edge>::const_iterator i = Y.begin(); i != Y.end(); i++) {
        D.removeFirst(*i); // del(iterator) is not defined for constant iterators
    }

    edge e;
    for(List<edge>::iterator i = D.begin(); i != D.end(); i++) {
        e = *i;
        List<edge> X; // (Indexes might be sufficient? Check later)
        GraphColoring coloring(G);
        X.pushBack(e);
        coloring[e->source()] = Color::RED;
        coloring[e->target()] = Color::BLUE;

        GenStage(Y, j, bonds, G, coloring, X, e->source(), e->target());
    }
}

int main(int argc, char* argv[])
{
    if (argc != 4 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        cout << "Usage: " << argv[0] << " <graph.csv> <cocircuit size bound> <components>" << endl;
        exit(2);
    }

    // TODO: Read from stdin
    ifstream fEdges(argv[1]);
    if (!fEdges.is_open()) {
        cerr << "Edges file doesn't exist or could not be accessed. Terminating." << endl;
        exit(3);
    }

    cutSizeBound = stoi(argv[2]);
    if (cutSizeBound < 1) {
        cerr << "Cocircuit size bound lower than 1. Terminating." << endl;
        exit(4);
    }

    components = stoi(argv[3]);
    if (cutSizeBound < 1) {
        cerr << "Desired components number lower than 1. Terminating." << endl;
        exit(5);
    }

    try {
        Graph G;
        csvToGraph(G, fEdges);

        List<List<edge>> bonds;
        List<edge> Y; // In the first stage (j = 1) Y = {}
        EscalatedCircuitCocircuit(G, Y, 1, bonds);

        List<List<edge>> jm1bonds, jbonds;
        jm1bonds = bonds;

        // The following works correctly only for j < 4 (as it accumulates bonds on one place and repeats the same procedure on already extended bonds)
        for (int j = 2; j < components; ++j) {
            for(List<List<edge> >::iterator it = jm1bonds.begin(); it != jm1bonds.end(); ++it) {
                EscalatedCircuitCocircuit(G, *it, j, jbonds);
            }
            jm1bonds = jbonds;
            jbonds.clear();
        }

        bonds = jm1bonds;

        for(List<List<edge> >::iterator it = bonds.begin(); it != bonds.end(); ++it) {
            cout << *it << endl;
        }

    } catch (invalid_argument *e) {
        cerr << "Error: " << e->what() << endl;
        return 1;
    }

    return 0;
}

