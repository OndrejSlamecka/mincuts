/*
 * Following the definitions from (yet unpublished) paper,
 * the matroid terms below correspond to the graph
 * terms right next to them.
 *
 *  Base - Spanning forest
 *  Circuit - either graph cycle or spanning forest formed by # of components - 2 trees
 *  Cocircuit - minimal edge-cut
 *  Hyperplane - maximal set not containing any basis (= complement of a min-cut)
 *
 * Note about coloring:
 *  Vertices: Red go all red, but vertices are blue only if they are in X
 *  Edges: All red or blue, acc. to subgraph
 */

#include "circuitcocircuit.h"
#include "helpers.h"

using namespace std;
using namespace ogdf;

ostream & operator<<(ostream &os, const bond &L)
{
    return os << L.edges;
}

void CircuitCocircuit::run(int k, List<bond> &bonds)
{    
    bond Y;
    GraphColoring coloring(G);
    extendBond(k, Y, coloring, 1, bonds);
}

void CircuitCocircuit::extendBond(int components, const bond &Y,
                                  GraphColoring &coloring, int j,
                                  List<bond> &bonds)
{
    // D is an arbitrary matroid base; our D corresponds to F from the paper now
    List<edge> D;
    minimalSpanningForest(components, Y, D);
    // Set D = E(F) \ Y... but it's already done, we've already forbidden Y

    List<bond> stageBonds;

    edge e;
    for(List<edge>::iterator i = D.begin(); i != D.end(); i++) {
        e = *i;

        bond X;
        X.edges.pushBack(e);
        X.lastBondFirstEdge = e;

        node u = e->source(), v = e->target();
        if (u->index() > v->index()) swap(u, v); // Def 4.3, i.

        coloring[u] = Color::RED;
        coloring.redNodes.pushBack(u);
        coloring[v] = Color::BLUE;

        genStage(components, Y, j, stageBonds, coloring, X);

        coloring.redNodes.clear();
        coloring[u] = Color::BLACK;
        coloring[v] = Color::BLACK;
    }

    if (j == components - 1) {        
        bonds.conc(stageBonds); // Beware, this empties stageBonds!
    } else {
        for(List<bond>::iterator it = stageBonds.begin(); it != stageBonds.end(); ++it) {
            extendBond(components, *it, coloring, j + 1, bonds);
        }
    }
}


void CircuitCocircuit::genStage(int components, const bond &Y,
                                int j, List<bond> &bonds,
                                GraphColoring &coloring, const bond &X)
{
    if (Y.edges.size() + X.edges.size() > cutSizeBound - components + j + 1) return;

    bond Ycopy(Y);
    bond XY(X); XY.edges.conc(Ycopy.edges); // G1 = G\XY, few parts require not having X and Y in the graph

    // Find set P = (a short circuit C in G1, s. t. |C ∩ X| = 1) \ X, G1 is G - Y
    node firstRed = NULL; // TODO: Needed?
    List<edge> P;
    shortestPath(coloring, XY.edges, firstRed, P);

    if (P.empty()) {
        // If there is no such path P above (line 6), then return ‘(j + 1) bond: Y union X’.
        bonds.pushBack(XY);
    } else {
        // Try adding each e in P to X

#ifdef DEBUG
        edge eg = P.front();
        if (coloring[eg->source()] != Color::RED && coloring[eg->target()] != Color::RED) {
            throw logic_error("GenStage: first edge of path has no end in red");
        }
#endif

        // Remember colours on the path before this iteration
        List<edge> blueBefore;

        forall_listiterators(edge, iterator, P) {
            edge e = *iterator;
            if (coloring[e] == Color::BLUE)
                blueBefore.pushBack(e);
        }

        // Color the path blue
        forall_listiterators(edge, iterator, P) {
            edge e = *iterator;
            coloring[e] = Color::BLUE;
        }

        List<edge> reconnectionBlues;

        // for each c ∈ D, recursively call GenCocircuits(X ∪ {c}).
        forall_listiterators(edge, iterator, P) {
            edge c = *iterator;
            node u, v; // c = (u,v), let u be red (and vertices on the path from the first red to u are red too)

            // By looking at the previous edge determine c = (u,v)
            // -- Check blue recoloring
            // If this is the first edge then just set u = firstRed, v = its opposite

            if (iterator == P.begin()) {
                u = firstRed;
                v = c->opposite(u);
            } else {
                edge prev = *iterator.pred();

                // The node which c and prev share is red
                if (c->source() == prev->source() || c->target() == prev->source()) {
                    u = prev->source();
                } else {
                    u = prev->target();
                }
                v = c->opposite(u);

                coloring.redNodes.pushBack(u);
                coloring[u] = Color::RED;
                if(   isBlueSubgraphDisconnected(coloring, X, c, u) // c is ignored because it is not colored black yet
                   && !reconnectBlueSubgraph(XY.edges, coloring, v, c, reconnectionBlues)) {
                    // Revert coloring so that the original coloring is used in the recursion level above
                    revertColoring(coloring, P, blueBefore, firstRed, reconnectionBlues, X);
                    return;
                }
            }

            if(!Y.edges.empty() && c->index() <= Y.lastBondFirstEdge->index()) {
                coloring[u] = Color::RED;
                coloring.redNodes.pushBack(u);
                coloring[c] = Color::RED;
                continue;
            }

            // all went fine, add c to X
            bond newX(X);
            newX.edges.pushBack(c);

            coloring[u] = Color::RED;
            coloring[v] = Color::BLUE;
            coloring.redNodes.pushBack(u);
            coloring[c] = Color::BLACK;

            genStage(components, Y, j, bonds, coloring, newX);
            coloring[c] = Color::RED;
        }

        // Revert coloring so that the original coloring is used in the recursion level above
        revertColoring(coloring, P, blueBefore, firstRed, reconnectionBlues, X);
    }
}



/**
 * Performs BFS to find the shortest path from s to t in graph G without using any edge from the forbidden edges.
 *
 */
void CircuitCocircuit::shortestPath(const GraphColoring &coloring, const List<edge> &XY,
                                    node &lastRed, List<edge> &path)
{
    Queue<node> Q;
    NodeArray<bool> visited(G, false);
    NodeArray<edge> accessEdge(G);

    node no;
    forall_nodes(no, G) {
        if (coloring[no] == Color::RED) {
            Q.append(no);
            visited[no] = true;
        }
    }
/*
    forall_listiterators(node, it, coloring.redNodes) {
        no = *it;
        Q.append(no);
        visited[no] = true;
    }
*/

    node u, v;
    edge e;
    while(!Q.empty()) {
        u = Q.pop();

        if (coloring[u] == Color::BLUE) { // Line6: path from any vertex of V_r to any of V_b
            for (node n = u; coloring[n] != Color::RED; n = v) {
                e = accessEdge[n];
                v = e->opposite(n);

                // Note that lastRed is red only when leaving the cycle (coloring[n] == RED)
                lastRed = v; // In reverse direction it is the first red...

                path.pushFront(e);
            }
            break;
        }

        forall_adj_edges(e, u) {
            if (XY.search(e).valid()) continue; // This is fast because m is a small constant

            v = e->opposite(u);
            if (!visited[v]) {
                accessEdge[v] = e;
                visited[v] = true;
                Q.append(v);
            }
        }
    }
}


/* --- Coloring, reconnecting blue subgraph --- */

void CircuitCocircuit::revertColoring(GraphColoring &coloring, List<edge> &edges,
                                      List<edge> &blueEdges, node firstRed,
                                      List<edge> &reconnectionBlues,
                                      const bond &X)
{
    forall_listiterators(edge, iterator, edges) {
        edge e = *iterator;

        coloring[e->source()] = Color::BLACK;
        coloring[e->target()] = Color::BLACK;
        coloring[e] = Color::BLACK;
    }

    forall_listiterators(edge, iterator, X.edges) {
        edge e = *iterator;

        if (coloring[e->source()] != Color::RED) coloring[e->source()] = Color::BLUE;
        if (coloring[e->target()] != Color::RED) coloring[e->target()] = Color::BLUE;
    }

    forall_listiterators(edge, iterator, reconnectionBlues) {
        edge e = *iterator;
        coloring[e] = Color::BLACK;
    }


    for(edge e : blueEdges) coloring[e] = Color::BLUE;
    coloring[firstRed] = Color::RED;
}

/**
 * @brief Runs DFS and hides everything which is blue and can be reached from start node
 * @param G
 * @param coloring
 * @param start
 */
void CircuitCocircuit::hideConnectedBlueSubgraph(const GraphColoring &coloring, node start)
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

/**
 * @brief Use BFS to find a connection of start with the rest of blue subgraph
 * @param coloring
 * @param start
 * @return bool
 */
bool CircuitCocircuit::findPathToAnyBlueAndColorItBlue(GraphColoring &coloring,
                                                       node start,
                                                       List<edge> &reconnectionBlues)
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
                coloring[e] = Color::BLUE;
                reconnectionBlues.pushBack(e);
            }
            return true;
        }

        forall_adj_edges(e, u) {
            v = e->opposite(u);

            if (coloring[v] == Color::RED) {
                continue;
            }

            if (!visited[v]) {
                visited[v] = true;
                accessEdge[v] = e;
                Q.append(v);
            }
        }
    }

    return false;
}

bool CircuitCocircuit::isBlueSubgraphDisconnected(GraphColoring &coloring, const bond &X, edge c, node u)
{
    G.hideEdge(c); // Don't consider c to be part of blue subgraph

    edge e;
    forall_adj_edges(e, u) {
        if (coloring[e] == Color::BLUE || X.edges.search(e).valid()) { // TODO: check if e.opposite(u) is blue, it's equiv. to X.edges.search(e).valid()
            G.restoreEdge(c);
            return true;
        }
    }

    G.restoreEdge(c);

    return false;
}


/**
 * Reconnects the blue subgraph if possible.
 * @return bool True for successful reconnection, false otherwise
 */
bool CircuitCocircuit::reconnectBlueSubgraph(const List<edge> &XY,
                                             GraphColoring &coloring, node u, edge c,
                                             ogdf::List<edge> &reconnectionBlues)
{        
    // enumerate one blue subgraph, call it G_b1
    // create graph G_rest = G \ X \ G_b1 and BFS (ignoring red edges) in it until blue is found
    //   -> path found, color it blue and continue
    //   -> path not found, fail here (return;)

    G.hideEdge(c);

    forall_listiterators(edge, it, XY) {
        G.hideEdge(*it);
    }

    // Hiding red subgraph with forall_edges(e, G) didn't work (e had no m_next set), instead red are forbidden in findPathToAnyBlue...
    // TODO: This will create problem if removing c doesn't disconnect the blue subgraph
    // Either change this (BFS for blue, then DFS and count found blue vertices... until we find all blue vertcies)
    // OR: Prove it cannot happen
    hideConnectedBlueSubgraph(coloring, u);

    // BFS in G from u to the first found blue edge
    //  -> not found => fail here
    //  -> path found => color it blue and continue

    if (!findPathToAnyBlueAndColorItBlue(coloring, u, reconnectionBlues)) {
        G.restoreAllEdges();
        return false;
    }

    G.restoreAllEdges();
    return true;
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
    // A modification of OGDF's makeMinimumSpanningTree

    NodeArray<int> setID(G);
    DisjointSets<> uf(G.numberOfNodes());
    for (node v = G.firstNode(); v; v = v->succ()) {
        setID[v] = uf.makeSet();
    }

    int stSize = 0;
    for (ListConstIterator<Prioritized<edge,int>> it = allEdgesSortedByIndex.begin(); it.valid(); ++it) {
        const edge e = (*it).item();

        if (Y.edges.search(e).valid()) continue;

        const int v = setID[e->source()];
        const int w = setID[e->target()];

        // See paper: cannonical, informal, bullet one
        if ((uf.find(v) != uf.find(w))
         && (Y.edges.empty() || e->index() > Y.lastBondFirstEdge->index())) {
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

