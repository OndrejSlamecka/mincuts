/*
 * Following the definitions from (yet unpublished) paper,
 * the matroid terms below correspond to the graph
 * terms right next to them.
 *
 *  Base - Spanning forest
 *  Circuit - either graph cycle or spanning forest formed by <# of components> - 2 trees
 *  Cocircuit - minimal edge-cut
 *  Hyperplane - maximal set not containing any basis (= complement of a min-cut)
 *
 * Note about colouring:
 *  Vertices: Red go all red, but vertices are blue only if they are in X
 *  Edges: All red or blue in red or blue subgraph respectively
 */

#include "circuitcocircuit.h"
#include "helpers.h"
#include <limits>

#ifdef MEASURE_RUNTIME
#include "runtimemeasurement.hpp"

RuntimeMeasurement rtm;

#define RTM_START RuntimeMeasurement::point start(rtm.mark(nBondsOutput));
#define RTM_END if (j <= measurementDepth) { rtm.log(j, nBondsOutput, start); }
#else
#define RTM_START
#define RTM_END
#endif

using namespace std;
using namespace ogdf;

ostream & operator<<(ostream &os, const bond &L)
{
    return os << L.edges;
}

#ifdef MEASURE_RUNTIME
CircuitCocircuit::CircuitCocircuit(ogdf::Graph &Graph, int cutSizeBound, int md)
    : G(Graph), cutSizeBound(cutSizeBound), lambda(G), measurementDepth(md)
#else
CircuitCocircuit::CircuitCocircuit(ogdf::Graph &Graph, int cutSizeBound)
    : G(Graph), cutSizeBound(cutSizeBound), lambda(G)
#endif
{
    // Sort edges by index for use by minimalSpanningForest
    for (ogdf::edge e = G.firstEdge(); e; e = e->succ()) {
        allEdgesSortedByIndex.pushBack(ogdf::Prioritized<ogdf::edge,int>(e, e->index()));
    }
    allEdgesSortedByIndex.quicksort();

#ifdef MEASURE_RUNTIME
    rtm = RuntimeMeasurement();
#endif
}

void CircuitCocircuit::run(int k, List<bond> &bonds)
{
    // Create map lambda : E(G) -> N (the natural numbers) for selection of the shortest
    // path. The map is randomized with each algorithm run in order to detect mistakes
    // related to graph traversing order.
    // |V| - k + 2 is the maximum length of a cycle, thus the maximum possible upper
    // bound on random numbers UB is such that (|V| - k + 2) * UB == max. long int
    std::default_random_engine engine(std::random_device{}());
    unsigned long int upper_bound = numeric_limits<unsigned long int>::max() / (G.numberOfNodes() - k + 2);
    std::uniform_int_distribution<unsigned long int> distribution(1, upper_bound);

    ogdf::edge e;
    forall_edges(e, G) {
        lambda[e] = distribution(engine);
    }

    // Run
    bond Y;
    extendBond(k, Y, 1, bonds);
}

void CircuitCocircuit::run(int k)
{
    List<bond> bonds;
    outputToStdout = true;
    run(k, bonds);
}

void CircuitCocircuit::extendBond(int components, const bond &Y, int j,
                                  List<bond> &bonds)
{
    // D is an arbitrary matroid base
    List<edge> D;
    minimalSpanningForest(components, Y, D);
    // Set D = E(F) \ Y... but it's already done, we've already forbidden Y

    GraphColouring colouring(G);

    forall_listiterators(edge, i, D) {
        edge e = *i;

        bond X;
        X.edges.pushBack(e);
        X.lastBondFirstEdge = e;

        node u = e->source(), v = e->target();
        if (u->index() > v->index()) swap(u, v); // Def 4.3, i.

        colouring.set(u, Colour::RED);
        colouring.set(v, Colour::BLUE);

        RTM_START
        genStage(colouring, components, Y, j, bonds, X);
        RTM_END

        colouring.set(u, Colour::BLACK);
        colouring.set(v, Colour::BLACK);
    }
}


void CircuitCocircuit::genStage(GraphColouring &colouring, int components, const bond &Y, int j,
                                List<bond> &bonds, const bond &X)
{
    if (Y.edges.size() + X.edges.size() > cutSizeBound - components + j + 1) return;

    // Find set P = (a short circuit C in G \ Y \ T_r, s. t. |C intersection X| = 1) \ X
    node firstRed = NULL;
    List<edge> P;
    shortestPath(colouring, Y.edges, X.edges, firstRed, P);

    if (P.empty()) {
        // If there is no such path P, then return ‘(j + 1) bond: Y union X’
        bond Ycopy(Y);
        bond XY(X); XY.edges.conc(Ycopy.edges);

        if (j == components - 1) {
            if (outputToStdout) {
                cout << XY << "\n";
            } else {
                bonds.pushBack(XY);
            }
#ifdef MEASURE_RUNTIME
        nBondsOutput += 1;
#endif
        } else {
            extendBond(components, XY, j + 1, bonds);
        }
    } else {
        // Try adding each c in P to X.

        List<edge> blueBefore;
        List<edge> newBlueTreeEdges;
        List<edge> oldBlueTreeEdges;

        // Colour the path blue but remember the previous colours
        forall_listiterators(edge, iterator, P) {
            edge e = *iterator;
            if (colouring[e] == Colour::BLUE) {
                blueBefore.pushBack(e);
            }
            colouring[e] = Colour::BLUE;
        }

        // c = (u,v), u is red, v is blue
        node u, v = firstRed; // we're doing u = v at the begining of each step

        // for each c in P, recursively call GenCocircuits(X union {c}).
        forall_listiterators(edge, iterator, P) {
            edge c = *iterator;

            // We're traversing P in order, so we can do this:
            u = v;
            v = c->opposite(u);

            // Colour as with u and v in X
            // (the colour of u has to be set red here in order to avoid being used
            // in a possible recreation of the blue tree, the colour of c cannot be
            // set here so that we can satisfy the input condition of the method
            // reCreateBlueTreeIfDisconnected
            colouring.set(v, Colour::BLUE);
            colouring.set(u, Colour::RED);

            // Check this condition before the slower hyperplane test
            if(c->index() <= X.lastBondFirstEdge->index()) {
                colouring[c] = Colour::RED;
                continue;
            }

            // Do we still have a hyperplane?
            if (   isBlueTreeDisconnected(colouring, c, u)
                && !reCreateBlueTreeIfDisconnected(colouring, Y.edges, X.edges, v, c, oldBlueTreeEdges, newBlueTreeEdges)) {
                // Revert colouring and end
                break;
            }

            // all went fine, add c to X
            bond newX(X);
            newX.edges.pushBack(c);

            // Don't set colour before we check for a hyperplane (this line could
            // actually be omitted, but we keep it to stick well with the theory)
            colouring[c] = Colour::BLACK;

            genStage(colouring, components, Y, j, bonds, newX);

            colouring[c] = Colour::RED;
        }

        // Revert colouring so that the original colouring is used in the recursion level above
        revertColouring(colouring, P, blueBefore, firstRed, X, oldBlueTreeEdges, newBlueTreeEdges);
    }
}

/**
 * Iota minimal path P_u,v is a path between vertices u,v which minimizes the vector of its indicies
 * (P_u,v[0].index, P_u,v[1].index,...)
 *
 * This function selects such path from two paths starting in s1 or s2, respectivelly (note s1 and s2
 * are blue) and returns one of s1 or s2.
 *
 * We expect both paths to be equally long
 */
node CircuitCocircuit::getStartNodeOfIotaMinimalPath(GraphColouring &colouring, NodeArray<edge> &accessEdge,
                                                     node s1, node s2)
{
    // Alternatively we could enumerate P1 and P2 and use lexicographical_compare on list of their indicies

    node n, m,
         a, b;
    edge e1, e2;
    node lexMinStartNode = NULL;

    for (n = s1, m = s2; colouring[n] != Colour::RED && colouring[m] != Colour::RED; n = a, m = b) {
        e1 = accessEdge[n];
        a = e1->opposite(n);

        e2 = accessEdge[m];
        b = e2->opposite(m);

        if (e1->index() < e2->index()) {
            lexMinStartNode = s1;
        } else if (e2->index() < e1->index()) {
            lexMinStartNode = s2;
        }
    }

    // One path hits red sooner -> they're not both equaly long which shows an error in shortestPath
    if (lexMinStartNode == NULL) {
        // This should never happen if this implementation is correct
        throw logic_error("Comparing index vector of two paths which are not of equal length");
    }

    return lexMinStartNode;
}

/**
 * Performs BFS to find the canonical shortest path from some red vertex to
 * some blue vertex in graph G without using any edge from X union Y.
 */
void CircuitCocircuit::shortestPath(GraphColouring &colouring, const List<edge> &Y, const List<edge> &X, node &lastRed, List<edge> &path)
{
    // For every path P = (e_0, e_1, e_2,...) we have the following triplet
    // (|P|, lambda_length(P), index_vector(P)), where |P| is number of its edges,
    // lambda length is the sum of lambda(e) through all e in P and index_vector
    // of P is the vector (e_0.index, e_1.index, e_2.index)
    //
    // In this function we're looking for such path P which (lexicographically)
    // minimizes this triplet. A modification of BFS is used.
	//
    // We compute the lambda length of paths by assigning the lambda distance
    // to nodes. Lambda distance ld is 0 for the starting nodes and when
    // discovering a new node v from node u (where e = {u, v}) then we set
    // ld[v] = ld[u] + lambda[e].
    //
    // Note that we only calculate the lambda distance of a node as we discover it
    // from path P1 and if we find an edge from P2 to the node then we only update
    // the lambda distance if |P1| = |P2|. This is because we don't care about
  	// the lambda distance primarily, our approach only computes the lambda length
	// correctly for the shortest (wrt. # of edges) paths currently discovered by
	// BFS.

    path.clear();

    Queue<node> Q;
    NodeArray<bool> visited(G, false);
    NodeArray<unsigned long int> lambdaDistance(G, 0);
    NodeArray<int> vertexDistance(G, 0);
    NodeArray<edge> accessEdge(G);

    node no;
    forall_listiterators(node, it, colouring.getRedVertices()) {
        no = *it;
        Q.append(no);
        visited[no] = true;
        lambdaDistance[no] = 0;
        vertexDistance[no] = 0;
    }

    forall_listiterators(edge, it, Y) {
        G.hideEdge(*it);
    }

    forall_listiterators(edge, it, X) {
        G.hideEdge(*it);
    }

    node foundBlue = NULL;
    node u, v;
    edge e;
    while(!Q.empty()) {
        u = Q.pop();

        if (colouring[u] == Colour::BLUE) { // Line6: path from any vertex of V_r to any of V_b
            if (foundBlue == NULL || lambdaDistance[u] < lambdaDistance[foundBlue]) {
                foundBlue = u;
            } else if (lambdaDistance[u] == lambdaDistance[foundBlue]) {
                // Compare path P1 = v-u and path P2 = v-foundBlue, and choose the one which
                // lexicographically minimizes its edge indicies vector
                // (P[0].index, P[1].index, P[2].index,...)

                // Note that by the start node we actually mean the blue node
                foundBlue = getStartNodeOfIotaMinimalPath(colouring, accessEdge, u, foundBlue);
            }
        }

        forall_adj_edges(e, u) {
            v = e->opposite(u);
            // The vertexDistance[u] == vertexDistance[v] - 1 condition is required in order
            // to prevent setting a wrong accessEdge (wrong as in wrong result of comparison
            // between paths which would be produced by switching the access edge)
            if (visited[v] && lambdaDistance[v] > lambdaDistance[u] + lambda[e] && vertexDistance[u] == vertexDistance[v] - 1) {
                lambdaDistance[v] = lambdaDistance[u] + lambda[e];
                accessEdge[v] = e;
            }

            if (!visited[v] && foundBlue == NULL) {
                accessEdge[v] = e;
                visited[v] = true;
                Q.append(v);
                lambdaDistance[v] = lambdaDistance[u] + lambda[e];
                vertexDistance[v] = vertexDistance[u] + 1;
            }
        }
    }

    if (foundBlue) {
        for (node n = foundBlue; colouring[n] != Colour::RED; n = v) {
            e = accessEdge[n];
            v = e->opposite(n);

            // Note that lastRed is red only when leaving the cycle (colouring[n] == RED)
            lastRed = v; // In reverse direction it is the first red...

            path.pushFront(e);
        }
    }

    G.restoreAllEdges();
}


/* --- Colouring, re-creating the blue tree --- */

bool CircuitCocircuit::isBlueTreeDisconnected(GraphColouring &colouring, edge c, node u)
{
    // We will test whether adding c to X would disconnect the blue tree
    edge e;

    G.hideEdge(c); // Don't consider c to be part of blue subgraph
    forall_adj_edges(e, u) {
        if (colouring[e] == Colour::BLUE) {
            G.restoreEdge(c);
            return true;
        }
    }

    G.restoreEdge(c);
    return false;
}

/**
 * Recolours only edges of course
 */
void CircuitCocircuit::recolourBlueTreeBlack(GraphColouring &colouring, node start, List<edge> &oldBlueTreeEdges)
{
    Stack<node> Q;
    NodeArray<bool> visited(G, false);

    Q.push(start);

    node u, v;
    edge e;
    while(!Q.empty()) {
        u = Q.pop();
        forall_adj_edges(e, u) {
            if (colouring[e] != Colour::BLUE) {
                continue;
            }
            v = e->opposite(u);
            if (!visited[v]) {
                visited[v] = true;
                Q.push(v);
            }
            colouring[e] = Colour::BLACK;
            oldBlueTreeEdges.pushBack(e);
        }
    }
}

/**
 * Returns true iff it was possible to re-create the blue tree such that it is connected
 * Expects graph with single blue tree as input (due to use of recolourBlueTreeBlack)
 */
bool CircuitCocircuit::reCreateBlueTreeIfDisconnected(GraphColouring &colouring, const List<edge> &Y, const List<edge> &X,
                                                      node v, edge c, List<edge> &oldBlueTreeEdges,
                                                      List<edge> &newBlueTreeEdges)
{
    // We will colour black what is blue and start building blue tree from scratch

    // * recolour both components of T_b \ c black
    // * run bfs from one blue vertex
    // * - for each found blue vertex colour the path to it blue, increase counter of found blue vertices
    // * - if all blue vertices weren't found then fail

    edge e;
    node m, // m is node currently being examined in BFS
         n, // neighbours of u
         a, b; // node currently being coloured on the path, its successor

    recolourBlueTreeBlack(colouring, v, oldBlueTreeEdges); // Recolours only edges of course, note that c is used now

    // Run BFS in G \ Y \ X \ T_r \ {c}, each time blue vertex x is found colour path v-x and increase nBlueVerticesFound
    // - if nBlueVerticesFound == colouring.nBlueVertices, return true
    // - if BFS ends and nBlueVerticesFound < colouring.nBlueVertices, then return true

    forall_listiterators(edge, it, Y) {
        G.hideEdge(*it);
    }

    forall_listiterators(edge, it, X) {
        G.hideEdge(*it);
    }
    G.hideEdge(c);

    Queue<node> Q;
    Q.append(v);
    int nBlueVerticesFound = 0; // v will be counted in the first iteration
    NodeArray<bool> visited(G, false);
    visited[v] = true;
    NodeArray<edge> accessEdge(G, NULL);

    while (!Q.empty()) {
        m = Q.pop();

        if (colouring[m] == Colour::BLUE) {
            nBlueVerticesFound++;

            // Colour path from v to u
            for (a = m; a != v; a = b) {
                e = accessEdge[a];
                b = e->opposite(a);

                colouring[e] = Colour::BLUE;
                newBlueTreeEdges.pushBack(e);
            }

            if (nBlueVerticesFound == colouring.nBlueVertices) {
                break;
            }
        }

        forall_adj_edges(e, m) {
            n = e->opposite(m);
            if (!visited[n] && colouring[n] != Colour::RED) {
                visited[n] = true;
                accessEdge[n] = e;
                Q.append(n);
            }
        }
    }

    G.restoreAllEdges();
    if (nBlueVerticesFound == colouring.nBlueVertices) {
        return true;
    } else {
        return false;
    }
}


void CircuitCocircuit::revertColouring(GraphColouring &colouring,
                                      List<edge> &P, List<edge> &blueEdgesOnP,
                                      node firstRed, const bond &X,
                                      List<edge> &oldBlueTreeEdges,
                                      List<edge> &newBlueTreeEdges)
{
    // The order is important here!
    forall_listiterators(edge, iterator, newBlueTreeEdges) {
        edge e = *iterator;
        colouring[e] = Colour::BLACK;
    }

    forall_listiterators(edge, iterator, oldBlueTreeEdges) {
        edge e = *iterator;
        colouring[e] = Colour::BLUE;
    }

    forall_listiterators(edge, iterator, P) {
        edge e = *iterator;

        colouring.set(e->source(), Colour::BLACK);
        colouring.set(e->target(), Colour::BLACK);
        colouring[e] = Colour::BLACK;
    }

    forall_listiterators(edge, iterator, X.edges) {
        edge e = *iterator;

        if (colouring[e->source()] != Colour::RED) colouring.set(e->source(), Colour::BLUE);
        if (colouring[e->target()] != Colour::RED) colouring.set(e->target(), Colour::BLUE);
    }

    for(edge e : blueEdgesOnP) {
        colouring[e] = Colour::BLUE;
    }

    colouring.set(firstRed, Colour::RED);
}

/* --- Minimal forest computation --- */

/**
 * @brief An implementation of Kruskal's algorithm to return minimal spanning forest on components-1 components.
 * @param components Number of components of result, generated forest will have one less!
 * @param forbidden Forbidden edges to use, typically existing bonds
 * @param result
 */
void CircuitCocircuit::minimalSpanningForest(int components, const bond &Y, List<edge> &result)
{
    // An implementation of Kruskal's algorithm taken from OGDF's makeMinimumSpanningTree

    NodeArray<int> setID(G);
    DisjointSets<> uf(G.numberOfNodes());
    for (node v = G.firstNode(); v; v = v->succ()) {
        setID[v] = uf.makeSet();
    }

    int stSize = 0;
    for (ListConstIterator<Prioritized<edge,int>> it = allEdgesSortedByIndex.begin(); it.valid(); ++it) {
        const edge e = (*it).item();

        const int v = setID[e->source()];
        const int w = setID[e->target()];

        // See paper: cannonical, informal, bullet one
        if ((uf.find(v) != uf.find(w))
         && (Y.edges.empty() || e->index() > Y.lastBondFirstEdge->index())
         && !Y.edges.search(e).valid()) {
            uf.link(uf.find(v), uf.find(w));
            result.pushBack(e);
            stSize++;
        }

        if (stSize == G.numberOfNodes() - components + 1) break; // Span. forest on n vertices and k-1 comp. has n-(k-1) edges
    }
}

CircuitCocircuit::~CircuitCocircuit()
{

}
