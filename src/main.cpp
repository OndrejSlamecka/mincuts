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
#include <ogdf/basic/BinaryHeap2.h>
#include <ogdf/basic/simple_graph_alg.h>

#include "helpers.hpp"
#include "graphcoloring.h"

using namespace std;
using namespace ogdf;

// Global variables are evil but these two never change...
int cutSizeBound;
int components;

/**
 * Performs BFS to find the shortest path from s to t in graph G without using any edge from the forbidden edges.
 *
 */
void shortestPath(const Graph &G, const GraphColoring &coloring, node s, node t,
                        const List<edge> &forbidden, node &lastRed, List<edge> &path)
{
    Queue<node> Q;

    NodeArray<bool> visited(G, false);
    NodeArray<edge> accessEdge(G);

    Q.append(s);
    visited[s] = true;

    node u, v;
    edge e;
    while(!Q.empty()) {
        u = Q.pop();

        if (u == t) {
            lastRed = s;
            for (node n = t; n != s; n = v) {
                e = accessEdge[n];
                v = e->opposite(n);

                if (coloring[n] == Color::RED) {
                    // In reverse direction it is the first red
                    lastRed = n;
                    break;
                }

                path.pushFront(e);
            }
            break;
        }

        forall_adj_edges(e, u) {
            if (forbidden.search(e).valid()) continue; // This is fast because m is a small constant

            v = e->opposite(u);
            if (!visited[v]) {
                accessEdge[v] = e;
                visited[v] = true;
                Q.append(v);
            }
        }
    }
}


/**
 * @brief Runs DFS and hides everything which can be reached from start node and is blue
 * @param G
 * @param coloring
 * @param start
 */
void hideConnectedBlueSubgraph(Graph &G, const GraphColoring &coloring, node start)
{
    Stack<node> Q;
    NodeArray<bool> visited(G, false);

    Q.push(start);

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


bool findPathToAnyBlueAndColorItBlue(const Graph &G, GraphColoring &coloring, node start)
{
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

#ifdef DEBUG
                if(coloring[n] == Color::RED || coloring[v] == Color::RED) {
                    throw logic_error("Red node in findPathToAnyblueAndColorItBlue");
                }
#endif

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
bool reconnectBlueSubgraph(Graph &G, const List<edge> &X,
                           GraphColoring &coloring, node u, node v, edge c)
{
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
        // G_b has been disconnected, hide X, G_r and one component of blue subgraph
        // (TODO: Maintain G_rest through the whole algorithm and not recompute here every time?)
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

void GenStage(const List<edge> &Y, int j, List<List<edge>> &bonds, Graph &G,
              GraphColoring &coloring, List<edge> X, node red, node blue)
{
    //if (Y.size() + X.size() > cutSizeBound - components + j + 1) return;
    if (Y.size() + X.size() > cutSizeBound) return;

    List<edge> Ycopy(Y);
    List<edge> XY(X); XY.conc(Ycopy); // G1 = G\XY, few parts require not having X and Y in the graph

    // Find set P = (a short circuit C in G1, s. t. |C ∩ X| = 1) \ X, G1 is G - Y
    node firstRed = NULL;
    List<edge> P;
    shortestPath(G, coloring, red, blue, XY, firstRed, P);

    if (P.size() > 0) {

#ifdef DEBUG
        edge eg = P.front();
        if (coloring[eg->source()] != Color::RED && coloring[eg->target()] != Color::RED) {
            throw logic_error("GenStage: first edge of path has no end in red");
        }
#endif

        // Color the path blue
        for(List<edge>::iterator iterator = P.begin(); iterator != P.end(); iterator++) {
            edge e = *iterator;

            coloring[e->source()] = Color::BLUE;
            coloring[e->target()] = Color::BLUE;
            coloring[e] = Color::BLUE;
        }
        coloring[firstRed] = Color::RED; // Keep red what was red

        // for each c ∈ D, recursively call GenCocircuits(X ∪ {c}).
        for(List<edge>::iterator iterator = P.begin(); iterator != P.end(); iterator++) {
            edge c = *iterator;

            if (!X.empty() && X.back()->index() > c->index()) // See paper, cannonical, informal, bullet one
                continue;

            List<edge> newX = X;
            newX.pushBack(c);
            coloring[c] = Color::BLACK;

            // Coloring red-c path red, determining u and v, coloring the rest blue
            node n1 = firstRed, n2 = blue, // after the first of the following for cycles these are the
                                      // nodes of the last red edge (one of them has to be incident with c)
                                      // initialized to red and blue since the first for can have 0 iterations
                 u, v; // c = (u,v), say u is red (and so will be all vertices on the path from the first red to u)

            // the following could be replaced by just coloring[previous c] and u determined by comparing previous c target/src with current c target/src
            for(List<edge>::iterator it2 = P.begin(); it2 != iterator; ++it2) {
                edge e = *it2;

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

            // If c = (u, v) is blue, reconnect blue subgraph if needed
            // (if u and v are both blue then find the shortest path
			// between them using only nonred edges)
            if (coloring[c] == Color::BLUE) { // FIX IT
                if(!reconnectBlueSubgraph(G, XY, coloring, u, v, c)) { // can't reconnect: fail
                    return;
                }
            }

            GenStage(Y, j, bonds, G, coloring, newX, u, v);
        }

        // Recolor to black what we colored red/blue so that the original coloring is used in the recursion level above
        for(List<edge>::iterator iterator = P.begin(); iterator != P.end(); iterator++) {
            edge e = *iterator;

            coloring[e->source()] = Color::BLACK;
            coloring[e->target()] = Color::BLACK;
            coloring[e] = Color::BLACK;
        }

        coloring[firstRed] = Color::RED;
        coloring[blue] = Color::BLUE;

    } else {
        // If there is no such path P above (line 6), then return ‘(j + 1) bond: Y union X’.
        bonds.pushBack(XY);
        //cout << XY << endl;
    }
}


// This is basically a copy of code from OGDF but with few simplifications

void minimalSpanningTree(const node start, BinaryHeap2<int, node> &pq,
                         NodeArray<int> &pqpos, NodeArray<edge> &pred,
                         NodeArray<bool> &processed)
{
    // insert start node
    int tmp(0);
    pq.insert(start, tmp, &pqpos[start]);

    // extract the nodes again along a minimum ST
    while (!pq.empty()) {
        const node v = pq.extractMin();
        processed[v] = true;
        for (adjEntry adj = v->firstAdj(); adj; adj = adj->succ()) {
            const node w = adj->twinNode();
            const edge e = adj->theEdge();
            const int wPos = pqpos[w];
            if (wPos == -1) {
                tmp = e->index();
                pq.insert(w, tmp, &pqpos[w]);

                pred[w] = e;
            } else if (!processed[w]
                    && e->index() < pq.getPriority(wPos)) {
                pq.decreaseKey(wPos, e->index());
                pred[w] = e;
            }
        }
    }

}

void minimalSpanningForest(const Graph &G, List<edge> &edges)
{
    BinaryHeap2<int, node> pq(G.numberOfNodes()); // priority queue of front vertices
    NodeArray<int> pqpos(G, -1); // position of each node in pq
    NodeArray<edge> pred(G);
    pred.init(G, NULL);

    NodeArray<bool> processed(G, false);

    int rootcount = 0, nedges = 0;
    for (node v = G.firstNode(); v; v = v->succ()) {
        if (!pred[v]) {
            ++rootcount;
            minimalSpanningTree(v, pq, pqpos, pred, processed);
        } else {
            edges.pushBack(pred[v]);
            ++nedges;

            // According to Line 1 in paper:
            // nedges == |V| - 1 - (comp. - 1)
            /*if(nedges == G.numberOfNodes() - 1 - (components - 1)) {
                break;
            }*/
        }
    }
}

void EscalatedCircuitCocircuit(Graph &G, const List<edge> &Y, int j, List<List<edge>> &bonds)
{
    List<edge> D;

    for(edge e : Y) G.hideEdge(e);
	// D is an arbitrary matroid base; our D corresponds to F from the paper now
    minimalSpanningForest(G, D);
    for(edge e : Y) G.restoreEdge(e);

    // Set D = E(F) \ Y... but it's already done, we've already forbidden Y

    List<List<edge>> stageBonds;

    edge e;
    for(List<edge>::iterator i = D.begin(); i != D.end(); i++) {
        e = *i;

		// See paper: cannonical, informal, bullet one
        if (!Y.empty() && Y.back()->index() > e->index())
            continue;

        List<edge> X; // (Indexes might be sufficient? Check later)
        GraphColoring coloring(G);
        X.pushBack(e);

        node u = e->source(), v = e->target();
        if (u->index() > v->index()) swap(u, v); // Def 4.3, i.

        coloring[u] = Color::RED;
        coloring[v] = Color::BLUE;

        GenStage(Y, j, stageBonds, G, coloring, X, u, v);

        coloring[u] = Color::BLACK;
        coloring[v] = Color::BLACK;
    }

    if (j < components) {
        for(List<List<edge>>::iterator it = stageBonds.begin(); it != stageBonds.end(); ++it) {
            EscalatedCircuitCocircuit(G, *it, j + 1, bonds);
        }
    }

    bonds.conc(stageBonds); // Beware, this empties stageBonds!
}


// bruteforce alg:

// note that these are actually variations
// TODO: some sort of generating combinations such that we get all of size s,
// s.t. 1 <= s <= k...
// IDEA: Generate all comb. of size i and in next iteration use those to build comb. of size i+1? Do later...
void combinations(Graph &G, const List<edge> &allEdges,
		const List<edge>::iterator &start, const List<edge> &acc,int k)
{
    int ncomponents;
    if (isMinCut(G, acc, ncomponents) == 0 && ncomponents <= components) {
		cout << acc << endl;
    }
	if (k == 0) {
		return;
	}

    // TODO: Improve
    List<edge> lessEdges(allEdges), moreAcc(acc);
    for (edge e : allEdges) {
//    for (List<edge>::iterator it = lessEdges.begin(); it != lessEdges.end(); ++it) {
//        edge e = *it;
        lessEdges.removeFirst(e);
		moreAcc.pushBack(e);

        combinations(G, lessEdges, start, moreAcc, k - 1);

		moreAcc.removeFirst(e);
        lessEdges.pushBack(e);
	}
}

void bruteforce(Graph &G)
{
	List<edge> allEdges;
	G.allEdges(allEdges);

	List<edge> acc;
	combinations(G, allEdges, allEdges.begin(), acc, cutSizeBound);
}

void printUsage(char *name) {
    cout << "Usage:\t" << name << " <edge_file.csv> " \
            "<cut size bound> <max components> [-bfc]" << endl;
	cout << endl \
   		 << "\t-bfc --\tuse bruteforcing of all combinations instead of\n" \
            "\t\tcircuit-cocircuit algorithm" << endl;
}

int main(int argc, char* argv[])
{
    if ((argc == 5 && argv[4] != string("-bfc"))
      || argc < 4 || argc > 5
	  || argv[1] == string("-h") || argv[1] == string("--help")) {
        printUsage(argv[0]);
        exit(1);
    }

    int algorithm = 0; // 0 for CircuitCocircuit, 1 for bruteforce
    if (argc == 5 && argv[4] == string("-bfc")) {
        algorithm = 1;
    }

    ifstream fEdges(argv[1]);
    if (!fEdges.is_open()) {
        cerr << "Edges file doesn't exist or could not be accessed. Terminating." << endl;
        exit(2);
    }

    try {
        cutSizeBound = stoi(argv[2]);
        components = stoi(argv[3]);
    } catch (invalid_argument &_) { // stoi failed
        printUsage(argv[0]);
        exit(1);
    };

    try {
        Graph G;
        csvToGraph(G, fEdges);

        if (algorithm == 1) {
            bruteforce(G);
        } else {
        	List<List<edge>> bonds;
            List<edge> Y; // In the first stage (j = 1) Y = {}
            EscalatedCircuitCocircuit(G, Y, 1, bonds);

			for(List<List<edge> >::iterator it = bonds.begin(); it != bonds.end(); ++it) {
				cout << *it << endl;
			}
        }

    } catch (invalid_argument &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}

