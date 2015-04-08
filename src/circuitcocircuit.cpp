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
    extendBond(k, Y, 1, bonds);
}

void CircuitCocircuit::extendBond(int components, const bond &Y, int j,
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

        coloring.set(u, Color::RED);
        coloring.set(v, Color::BLUE);

        genStage(components, Y, j, stageBonds, X);

        coloring.set(u, Color::BLACK);
        coloring.set(v, Color::BLACK);
    }

    if (j == components - 1) {        
        bonds.conc(stageBonds); // Beware, this empties stageBonds!
    } else {
        for(List<bond>::iterator it = stageBonds.begin(); it != stageBonds.end(); ++it) {
            extendBond(components, *it, j + 1, bonds);
        }
    }
}


void CircuitCocircuit::genStage(int components, const bond &Y, int j,
                                List<bond> &bonds, const bond &X)
{
    if (Y.edges.size() + X.edges.size() > cutSizeBound - components + j + 1) return;

    bond Ycopy(Y);
    bond XY(X); XY.edges.conc(Ycopy.edges); // G1 = G\XY, few parts require not having X and Y in the graph

    // Find set P = (a short circuit C in G1, s. t. |C ∩ X| = 1) \ X, G1 is G - Y
    node firstRed = NULL;
    List<edge> P;
    shortestPath(XY.edges, firstRed, P);

    if (P.empty()) {
        // If there is no such path P, then return ‘(j + 1) bond: Y union X’.
        bonds.pushBack(XY);
    } else {
        // Try adding each c in P to X.

        List<edge> blueBefore;
        List<edge> newBlueTreeEdges;
        List<edge> oldBlueTreeEdges;

        // Colour the path blue but remember previous colours
        forall_listiterators(edge, iterator, P) {
            edge e = *iterator;
            if (coloring[e] == Color::BLUE) {
                blueBefore.pushBack(e);
            }
            coloring[e] = Color::BLUE;
        }        

        // c = (u,v), u is red, v is blue
        node u, v = firstRed; // we're doing u = v at the begining of each step

        // for each c ∈ P, recursively call GenCocircuits(X ∪ {c}).
        forall_listiterators(edge, iterator, P) {
            edge c = *iterator;            

            // We're traversing P in order, so we can do this:
            u = v;
            v = c->opposite(u);

            // Add u to red subgraph (this has to be done here so that it won't be used in possible reconnection)
            coloring.set(u, Color::RED);

            // RECREATION OF BLUE TREE
            // * recolors both components of T_b \ c black
            // * runs bfs from one blue vertex
            // * - mark each found blue vertex and color path to it
            // * - if all blue vertices weren't found fail

            if (   isBlueTreeDisconnected(c, u)
                && !recreateBlueTreeIfDisconnected(XY.edges, v, c, oldBlueTreeEdges, newBlueTreeEdges)) {
                revertColoring(P, blueBefore, firstRed, X, oldBlueTreeEdges, newBlueTreeEdges);
                return;
            }

            if(   (!Y.edges.empty() && c->index() <= Y.lastBondFirstEdge->index())
               || (                    c->index() <= X.lastBondFirstEdge->index())) {
                coloring[c] = Color::RED;
                continue;
            }

            // all went fine, add c to X
            bond newX(X);
            newX.edges.pushBack(c);

            coloring.set(v, Color::BLUE);
            coloring[c] = Color::BLACK;

            genStage(components, Y, j, bonds, newX);

            coloring[c] = Color::RED;
        }

        // Revert coloring so that the original coloring is used in the recursion level above
        revertColoring(P, blueBefore, firstRed, X, oldBlueTreeEdges, newBlueTreeEdges);
    }
}

/**
 * Returns the starting node of path P which lexicographically minimizes the vector (P[0].index, P[1].index,...)
 * Note that by the start node we actually mean the blue node
 */
node CircuitCocircuit::lexicographicallyMinimalPathStartNode(NodeArray<edge> &accessEdge, node s1, node s2)
{
    // Alternatively we could enumerate P1 and P2 and use lexicographical_compare on list of their indicies

    node n, m,
         a, b;
    edge e1, e2;
    node lexMinStartNode = NULL;

    for (n = s1, m = s2; coloring[n] != Color::RED && coloring[m] != Color::RED; n = a, m = b) {
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

    // One path hits red -> it is shorter -> lexicographically less than the longer one
    if (lexMinStartNode == NULL) {
        if (coloring[n] == Color::RED) {
            lexMinStartNode = s1;
        } else if (coloring[m] == Color::RED) {
            lexMinStartNode = s2;
        } else {
            // This should never happen if this implementation is correct
            throw logic_error("Could not lexicographically compare two paths with equal lambda length.");
        }
    }

    return lexMinStartNode;
}

/**
 * Performs BFS to find the shortest path from s to t in graph G without using any edge from the forbidden edges.
 *
 */
void CircuitCocircuit::shortestPath(const List<edge> &XY, node &lastRed, List<edge> &path)
{
    // TODO: Comment this method
    path.clear();

    Queue<node> Q;
    NodeArray<bool> visited(G, false);
    NodeArray<int> distance(G, -1);
    NodeArray<int> vertexDistance(G, -1);
    NodeArray<edge> accessEdge(G);

    node no;
    forall_nodes(no, G) {
        if (coloring[no] == Color::RED) {
            Q.append(no);
            visited[no] = true;
            distance[no] = 0;
            vertexDistance[no] = 0;
        }
    }
/*
    forall_listiterators(node, it, coloring.redNodes) {
        no = *it;
        Q.append(no);
        visited[no] = true;
    }
*/

    forall_listiterators(edge, it, XY) {
        G.hideEdge(*it);
    }

    node foundBlue = NULL;
    node u, v;
    edge e;
    while(!Q.empty()) {
        u = Q.pop();

        if (coloring[u] == Color::BLUE) { // Line6: path from any vertex of V_r to any of V_b
            if (foundBlue == NULL || distance[u] < distance[foundBlue]) {
                foundBlue = u;
            } else if (distance[u] == distance[foundBlue]) {
                // Compare path P1 = v-u and path P2 = v-foundBlue,
                // such that we get the one which lexicographically minimizes
                // vector (P[0].index, P[1].index, P[2].index,...)

                // Note that by the start node we actually mean the blue node
                foundBlue = lexicographicallyMinimalPathStartNode(accessEdge, u, foundBlue);
            }
        }

        forall_adj_edges(e, u) {
            v = e->opposite(u);
            if (visited[v] && distance[v] > distance[u] + lambda[e] && vertexDistance[u] == vertexDistance[v] - 1) {
                distance[v] = distance[u] + lambda[e];
                accessEdge[v] = e;
            }

            if (!visited[v] && foundBlue == NULL) {
                accessEdge[v] = e;
                visited[v] = true;
                Q.append(v);
                distance[v] = distance[u] + lambda[e];
                vertexDistance[v] = vertexDistance[u] + 1;
            }
        }
    }

    if (foundBlue) {
        for (node n = foundBlue; coloring[n] != Color::RED; n = v) {
            e = accessEdge[n];
            v = e->opposite(n);

            // Note that lastRed is red only when leaving the cycle (coloring[n] == RED)
            lastRed = v; // In reverse direction it is the first red...

            path.pushFront(e);
        }
    }

    G.restoreAllEdges();
}


/* --- Coloring, reconnecting blue subgraph --- */

bool CircuitCocircuit::isBlueTreeDisconnected(edge c, node u)
{
    // We will test whether adding c to X would disconnect the blue tree
    edge e;

    G.hideEdge(c); // Don't consider c to be part of blue subgraph
    forall_adj_edges(e, u) {
        if (coloring[e] == Color::BLUE) {
            G.restoreEdge(c);
            return true;
        }
    }

    G.restoreEdge(c);
    return false;
}

/**
 * Recolors only edges of course
 */
void CircuitCocircuit::recolorBlueTreeBlack(node start, List<edge> &oldBlueTreeEdges)
{
    Stack<node> Q;
    NodeArray<bool> visited(G, false);

    Q.push(start);

    node u, v;
    edge e;
    while(!Q.empty()) {
        u = Q.pop();
        forall_adj_edges(e, u) {
            if (coloring[e] != Color::BLUE) {
                continue;
            }
            v = e->opposite(u);
            if (!visited[v]) {
                visited[v] = true;
                Q.push(v);
            }
            coloring[e] = Color::BLACK;
            oldBlueTreeEdges.pushBack(e);
        }
    }
}

/**
 * Returns true iff blue tree was not disconnected or if it was possible to recreate it such that it is connected
 */
bool CircuitCocircuit::recreateBlueTreeIfDisconnected(const List<edge> &XY, node v, edge c,
                                                      List<edge> &oldBlueTreeEdges, List<edge> &newBlueTreeEdges)
{
    // We will colour black what is blue and start building blue tree from scratch

    edge e;
    node m, // m is node currently being examined in BFS
         n, // neighbours of u
         a, b; // node currently being coloured on the path, its successor

    recolorBlueTreeBlack(v, oldBlueTreeEdges); // Recolors only edges of course, note that c is used now

    // Run BFS in G \ XY \ T_r \ {c}, each time blue vertex x is found colour path v-x and increase nBlueVerticesFound
    // - if nBlueVerticesFound == coloring.nBlueVertices, return true
    // - if BFS ends and nBlueVerticesFound < coloring.nBlueVertices, then return true

    forall_listiterators(edge, it, XY) {
        G.hideEdge(*it);
    }
    G.hideEdge(c);

    Queue<node> Q;
    Q.append(v);
    int nBlueVerticesFound = 0; // v will be counted in first iteration
    NodeArray<bool> visited(G, false);
    visited[v] = true;
    NodeArray<edge> accessEdge(G, NULL);

    while (!Q.empty()) {
        m = Q.pop();

        if (coloring[m] == Color::BLUE) {
            nBlueVerticesFound++;

            // Color path from v to u
            for (a = m; a != v; a = b) {
                e = accessEdge[a];
                b = e->opposite(a);

                coloring[e] = Color::BLUE;
                newBlueTreeEdges.pushBack(e);
            }

            if (nBlueVerticesFound == coloring.nBlueVertices) {
                break;
            }
        }

        forall_adj_edges(e, m) {
            n = e->opposite(m);
            if (!visited[n] && coloring[n] != Color::RED) {
                visited[n] = true;
                accessEdge[n] = e;
                Q.append(n);
            }
        }
    }

    G.restoreAllEdges();
    if (nBlueVerticesFound == coloring.nBlueVertices) {
        return true;
    } else {
        return false;
    }
}


void CircuitCocircuit::revertColoring(List<edge> &P, List<edge> &blueEdgesOnP,
                                      node firstRed, const bond &X,
                                      List<edge> &oldBlueTreeEdges,
                                      List<edge> &newBlueTreeEdges)
{
    // The order here is important!
    forall_listiterators(edge, iterator, newBlueTreeEdges) {
        edge e = *iterator;
        coloring[e] = Color::BLACK;
    }

    forall_listiterators(edge, iterator, oldBlueTreeEdges) {
        edge e = *iterator;
        coloring[e] = Color::BLUE;
    }

    forall_listiterators(edge, iterator, P) {
        edge e = *iterator;

        coloring.set(e->source(), Color::BLACK);
        coloring.set(e->target(), Color::BLACK);
        coloring[e] = Color::BLACK;
    }

    forall_listiterators(edge, iterator, X.edges) {
        edge e = *iterator;

        if (coloring[e->source()] != Color::RED) coloring.set(e->source(), Color::BLUE);
        if (coloring[e->target()] != Color::RED) coloring.set(e->target(), Color::BLUE);
    }

    for(edge e : blueEdgesOnP) {
        coloring[e] = Color::BLUE;
    }

    coloring.set(firstRed, Color::RED);
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

