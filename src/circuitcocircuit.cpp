/*
 * Copyright (c) 2015, Ondrej Slamecka <ondrej@slamecka.cz>
 * See the LICENSE file in the root folder of this repository.
 */

/*
 * Note about colouring:
 *  Vertices: Red go all red, but vertices are blue only if they are in X
 *  Edges: All red or blue in red or blue subgraph respectively
 */

#include "./circuitcocircuit.h"
#include <limits>
#include <cstdint>
#include <algorithm>  // swap
#include "./helpers.h"

#ifdef MEASURE_RUNTIME
#include "./runtimemeasurement.h"

RuntimeMeasurement rtm;

#define RTM_START RuntimeMeasurement::point start(rtm.mark(nBondsOutput));
#define RTM_END if (j <= measurementDepth) { rtm.log(j, nBondsOutput, start); }
#else
#define RTM_START
#define RTM_END
#endif

using std::ostream; using std::invalid_argument; using std::logic_error;
using std::cout; using std::cerr; using std::endl;
using ogdf::edge; using ogdf::node; using ogdf::adjEntry;
using ogdf::ListConstIterator;
using ogdf::Stack; using ogdf::NodeArray;
using ogdf::Queue; using ogdf::Prioritized; using ogdf::DisjointSets;
using ogdf::List; using ogdf::ListConstIterator;

ostream & operator<<(ostream &os, const bond &L) {
    return os << L.edges;
}

#ifdef MEASURE_RUNTIME
CircuitCocircuit::CircuitCocircuit(DCGraph &Graph, int cutSizeBound, int md)
    : G(Graph), dyncon(G), cutSizeBound(cutSizeBound), lambda(G),
    measurementDepth(md) {
#else
CircuitCocircuit::CircuitCocircuit(DCGraph &Graph, int cutSizeBound)
    : G(Graph), dyncon(G), cutSizeBound(cutSizeBound), lambda(G) {
#endif
    // Sort edges by index for use in minimalSpanningForest
    for (edge e : G.edges) {
        allEdgesSortedByIndex.pushBack(Prioritized<edge, int>(e, e->index()));
    }
    allEdgesSortedByIndex.quicksort();

#ifdef MEASURE_RUNTIME
    rtm = RuntimeMeasurement();
#endif
}

void CircuitCocircuit::run(int k, List<bond> &bonds) {
    // Create map lambda : E(G) -> N (the natural numbers) for selection of
    // the unique shortest path. The map is randomized with each algorithm run
    // in order to detect mistakes related to graph traversing order.
    // |V| - k + 2 is the maximum length of a cycle, thus the maximum possible
    // length of a path is |V| - k + 1
    // (|V| - k + 1) * UB == max. long int
    std::random_device rd;
    std::default_random_engine engine(rd());
    constexpr uint64_t max_uint64_t = std::numeric_limits<uint64_t>::max();
    uint64_t upper_bound = max_uint64_t / (G.numberOfNodes() - k + 1);
    std::uniform_int_distribution<uint64_t> distribution(1, upper_bound);

    for (edge e : G.edges) {
        lambda[e] = distribution(engine);
    }

    // Run
    bond Y;
    List<edge> redEdges;
    extendBond(k, Y, 1, bonds, redEdges);
}

void CircuitCocircuit::run(int k) {
    List<bond> bonds;
    outputToStdout = true;
    run(k, bonds);
}

void CircuitCocircuit::extendBond(int components, const bond &Y, int j,
                                  List<bond> &bonds, List<edge> &redEdgesLevelAbove) {
    GraphColouring colouring(G);

    for (edge e : redEdgesLevelAbove) {
        dyncon.ins(e);
    }

    // D is an arbitrary matroid base
    List<edge> D;
    minimalSpanningForest(components, Y, D);

    for (edge e : D) {
        if ((!Y.edges.empty() && e->index() < Y.lastBondFirstEdge->index())) {
            continue;
        }

        bond X;
        X.edges.pushBack(e);
        X.lastBondFirstEdge = e;
        dyncon.del(e);

        node u = e->source(), v = e->target();
        if (u->index() > v->index()) swap(u, v);  // Definition 4.1.a

        colouring.set(u, Colour::RED);
        colouring.set(v, Colour::BLUE);

        List<edge> redEdges; // New for each started branch of computation

        RTM_START
        genStage(colouring, components, Y, j, bonds, X, redEdges);
        RTM_END

        colouring.set(u, Colour::BLACK);
        colouring.set(v, Colour::BLACK);
        dyncon.ins(e);
        // Here e is removed from X (as new X is created with each
        // iteration there does not have to be X.del(e))
    }

    for (edge e : redEdgesLevelAbove) {
        dyncon.del(e);
    }
}


void CircuitCocircuit::genStage(GraphColouring &colouring, int components,
                                const bond &Y, int j, List<bond> &bonds,
                                const bond &X, List<edge> &redEdges) {
    if (Y.edges.size() + X.edges.size() > cutSizeBound - components + j + 1) {
        return;
    }

    // Find the path P minimizing (|P|, lambda_length(P), index_vector(P))
    // See shortestPath for more info
    node firstRed = NULL;
    List<edge> P;
    shortestPath(colouring, Y.edges, X.edges, firstRed, P);

#ifdef MEASURE_PATHS_LENGTHS
    cout << j << " " << X.edges.size() << " " << P.size() << endl;
#endif

    if (P.empty()) {
        // If there is no such path P, then return ‘(j + 1) bond: Y union X’
        bond XY;
        XY.lastBondFirstEdge = X.lastBondFirstEdge;
        List<edge> Yedges(Y.edges); XY.edges.conc(Yedges);
        List<edge> Xedges(X.edges); XY.edges.conc(Xedges);

        if (j == components - 1) {
            if (outputToStdout) {
#ifndef MEASURE_PATHS_LENGTHS
                cout << XY << "\n";
#endif
            } else {
                bonds.pushBack(XY);
            }
#ifdef MEASURE_RUNTIME
        nBondsOutput += 1;
#endif
        } else {
            extendBond(components, XY, j + 1, bonds, redEdges);
        }
    } else {
        // Try adding each c in P to X.

        // c = (u,v), u is red, v is blue
        node u, v = firstRed;  // we're doing u = v at the begining of each step

        // for each c in P, recursively call GenCocircuits(X union {c}).
        List<edge> traversed;
        for (edge c : P) {
            // We're traversing P in order, so we can do this:
            u = v;
            v = c->opposite(u);
            //cout << "Path edge: " << c->index() << " of path: " << edgelist2str(P) << endl;

            // Colour as with u and v in X
            // (the colour of u has to be set red here in order to avoid being
            // used in a possible re-creation of the blue tree, the colour of c
            // cannot be set here so that we can satisfy the input condition of
            // the method reCreateBlueTreeIfDisconnected
            colouring.set(v, Colour::BLUE);
            colouring.set(u, Colour::RED);

            // Do we still have a hyperplane?
            if (!dyncon.connected(u, v)) {
                break;
            }

            traversed.pushBack(c);

            // This condition has to be checked after the hyperplane test!
            if (c->index() <= X.lastBondFirstEdge->index()) {
                colouring[c] = Colour::RED;
                redEdges.pushBack(c);
                dyncon.del(c);
                continue;
            }

            // all went fine, add c to X
            bond newX(X);
            newX.edges.pushBack(c);

            // Don't set colour before we check for a hyperplane
            colouring[c] = Colour::BLACK;

            // c is lost for connectivity in this branch of computation
            // (now it will be in X, then red)
            dyncon.del(c);

            genStage(colouring, components, Y, j, bonds, newX, redEdges);

            colouring[c] = Colour::RED;
            redEdges.pushBack(c);
        }

        // Revert colouring so that the original colouring is used in the
        // recursion level above
        revertColouring(colouring, P, firstRed, X);

        // Put the path back to dyncon (the path cannot contain edges in
        // X or red so this is safe to do)
        // TODO: How about multigraphs being created here?
        // E.g. dyncon replaces tree edge with some e in P and then we
        // again add it here as non-tree -- isn't it causing problems
        // for the dyncon alg?
        for (edge e : traversed) {
            redEdges.popBack();
            dyncon.ins(e);
        }
    }
}

/**
 * Iota minimal path P_{u,v} is a path between vertices u,v which minimizes
 * the vector of its indicies (P_{u,v}[0].index, P_{u,v}[1].index,...)
 * This function selects such path from two paths starting in s1 or s2,
 * respectivelly (note s1 and s2 are blue) and returns one of s1 or s2.
 *
 * Both paths are expected to be equally long.
 */
node CircuitCocircuit::getStartNodeOfIotaMinimalPath(GraphColouring &colouring,
                                                     NodeArray<edge> &accessEdge,
                                                     node s1, node s2) {
    // Alternatively we could enumerate P1 and P2 and use
    // lexicographical_compare on list of their indicies

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

    // One path hits red sooner -> they're not both equaly long which shows
    // an error in shortestPath implementation
    if (lexMinStartNode == NULL) {
        // This should never happen if this implementation is correct
        throw logic_error("Comparing index vector of two paths which are " \
                          "not of equal length.");
    }

    return lexMinStartNode;
}

/**
 * Performs BFS to find the canonical shortest path from some red vertex to
 * some blue vertex in graph G without using any edge from X union Y.
 */
void CircuitCocircuit::shortestPath(GraphColouring &colouring,
                                    const List<edge> &Y, const List<edge> &X,
                                    node &lastRed, List<edge> &path) {
    // For every path P = (e_0, e_1, e_2,...) we have the following triplet
    // (|P|, lambda_length(P), index_vector(P)), where |P| is number of its
    // edges, lambda length is the sum of lambda(e) through all e in P and
    // index_vector of P is the vector (e_0.index, e_1.index, e_2.index)
    //
    // In this function we're looking for such path P which (lexicographically)
    // minimizes this triplet. A modification of BFS is used.
    //
    // We compute the lambda length of paths by assigning the lambda distance
    // to nodes. Lambda distance ld is 0 for the starting nodes and when
    // discovering a new node v from node u (where e = {u, v}) then we set
    // ld[v] = ld[u] + lambda[e]. If we can arrive to v from a different node
    // w (which is in the same distance from the start as u) using edge f and
    // with ld[w] + lambda[f] < ld[v] then we use f as an access edge and set
    // ld[v] = ld[w] + lambda[f].
    //
    // Note that we only calculate the lambda distance of a node as we discover
    // it from path P1 and if we find an edge from P2 to the node then we only
    // update the lambda distance if it would decrease and if |P1| = |P2|. This
    // is because we don't care about the lambda distance primarily, our
    // approach only computes the lambda length correctly for the shortest
    // (wrt. # of edges) paths currently discovered by BFS.

    path.clear();

    Queue<node> Q;
    NodeArray<bool> visited(G, false);
    NodeArray<u_int64_t> lambdaDistance(G, 0);
    NodeArray<int> vertexDistance(G, 0);
    NodeArray<edge> accessEdge(G);

    // Hide Y and X
    DCGraph::HiddenEdgeSet hidden_xy(G);

    for (edge e : Y) {
        hidden_xy.hide(e);
    }

    for (edge e : X) {
        hidden_xy.hide(e);
    }

    // Init
    for (node n : colouring.getRedVertices()) {
        Q.append(n);
        visited[n] = true;
        lambdaDistance[n] = 0;
        vertexDistance[n] = 0;
    }

    // Start BFS
    node foundBlue = NULL;
    node u, v;
    while (!Q.empty()) {
        u = Q.pop();

        if (colouring[u] == Colour::BLUE) {
            if (foundBlue == NULL || lambdaDistance[u] < lambdaDistance[foundBlue]) {
                foundBlue = u;
            } else if (lambdaDistance[u] == lambdaDistance[foundBlue]) {
                // Compare path P1 = v-u and path P2 = v-foundBlue, and choose
                // the one which lexicographically minimizes its edge indicies
                // vector (P[0].index, P[1].index, P[2].index,...)

                // Note that by the start node we actually mean the blue node
                foundBlue = getStartNodeOfIotaMinimalPath(colouring,
                                                          accessEdge, u,
                                                          foundBlue);
            }
        }

        for (adjEntry adj : u->adjEntries) {
            edge e = adj->theEdge();
            v = e->opposite(u);
            // The vertexDistance[u] == vertexDistance[v] - 1 condition is
            // required in order to prevent setting a wrong accessEdge
            // (wrong as in wrong result of comparison between paths which
            // would be produced by switching the access edge)
            // This condition can't be replaced by v == foundBlue (a simple
            // counterexample can be shown)
            if (visited[v] && vertexDistance[u] == vertexDistance[v] - 1
                    && lambdaDistance[v] > lambdaDistance[u] + lambda[e]) {
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
            edge e = accessEdge[n];
            v = e->opposite(n);

            // Note that lastRed is set correctly only in the iteration
            // when colouring[n] == RED is satisfied (which is the last iter.)
            lastRed = v;  // In reverse direction it is the first red...

            path.pushFront(e);
        }
    }

    hidden_xy.restore();
}


/* --- Colouring --- */

void CircuitCocircuit::revertColouring(GraphColouring &colouring,
                                      List<edge> &P, node firstRed,
                                      const bond &X) {
    // The order is important here!
    for (edge e : P) {
        colouring.set(e->source(), Colour::BLACK);
        colouring.set(e->target(), Colour::BLACK);
        colouring[e] = Colour::BLACK;
    }

    // TODO: Is the rest necessary?

    for (edge e : X.edges) {
        if (colouring[e->source()] != Colour::RED) {
            colouring.set(e->source(), Colour::BLUE);
        }
        if (colouring[e->target()] != Colour::RED) {
            colouring.set(e->target(), Colour::BLUE);
        }
    }

    colouring.set(firstRed, Colour::RED);
}

/* --- Minimal forest computation --- */

/**
 * @brief An implementation of Kruskal's algorithm to return minimal spanning
 *        forest on (components-1) components.
 * @param components The k in k-bonds
 * @param forbidden Forbidden edges to use, typically existing bonds
 * @param result
 */
void CircuitCocircuit::minimalSpanningForest(int components, const bond &Y,
                                             List<edge> &result) {
    // An modified implementation of Kruskal's algorithm
    // taken from OGDF's makeMinimumSpanningTree

    NodeArray<int> setID(G);
    DisjointSets<> uf(G.numberOfNodes());
    for (node v = G.firstNode(); v; v = v->succ()) {
        setID[v] = uf.makeSet();
    }

    int stSize = 0;
    for (ListConstIterator<Prioritized<edge, int>> it = allEdgesSortedByIndex.begin(); it.valid(); ++it) {
        const edge e = (*it).item();

        const int v = setID[e->source()];
        const int w = setID[e->target()];

        if ((uf.find(v) != uf.find(w))
         && !Y.edges.search(e).valid()) {  // Faster than characteristic vector
            uf.link(uf.find(v), uf.find(w));
            result.pushBack(e);
            stSize++;
        }

        // Span. forest on n vertices and k-1 comp. has n-(k-1) edges
        if (stSize == G.numberOfNodes() - components + 1) break;
    }
}

CircuitCocircuit::~CircuitCocircuit() { }
