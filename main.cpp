#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <set>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/graph_generators.h>
#include <ogdf/layered/DfsAcyclicSubgraph.h>
#include <ogdf/basic/Queue.h>

using namespace std;
using namespace ogdf;

int m = 3; // Cocircuit size bound


template <template <class> class C, typename T>
bool contains(C<T> container, T item) {
    return std::find(container.begin(), container.end(), item) != container.end();
}

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
        G.newEdge(nodes[v], nodes[u], id);
    }

    return G;
}

/**
 * Performs BFS to find the shortest path from s to t in graph g without using any edge from the forbidden edges.
 * Returns empty set if no such path exists.
 */
List<edge> shortestPath(const Graph &g, node s, node t, List<edge> forbidden) {
    List<edge> path;
    node v, u;
    edge e;

    Queue<node> Q;

    NodeArray<node> predecessor(g);
    NodeArray<bool> visited(g);
    forall_nodes(v, g) visited[v] = false;

    Q.append(s);
    visited[s] = true;

    while(!Q.empty()) {
        v = Q.pop();

        if (v == t) {
            // traceback predecessors and reconstruct path
            for (node n = t; t != s; n = predecessor[n]) {
                path.pushBack(g.searchEdge(n, predecessor[n]));
            }
            break;
        }

        forall_adj_edges(e, v) {
            if (forbidden.search(e).valid()) continue; // TODO: Use BST or array (id -> bool) to represent forbidden?

            u = e->target();
            if (!visited[u]) {
                predecessor[u] = v; // TODO: Unite predecessor and visited?
                visited[u] = true;
                Q.append(u);
            }
        }
    }

    return path;
}

set<edge> GenCocircuits(const Graph &g, set<edge> X, NodeArray<int> coloring) {
    // if(E\X contains no hyperplane of M || X.size() > m)
        // return NULL;

    // Find set D = (a short circuit C in G, s. t. |C ∩ X| = 1) \ X
    Array<edge> D = shortestPath(g, s, t, X);
    if (D.size() > 0) { // can't use size
        // for each c ∈ D, recursively call GenCocircuits(X ∪ {c}).
    } else {
        // If there is no such circuit C above (line 4), then return ‘Cocircuit: X’.
    }
}


int main()
{
    DfsAcyclicSubgraph DfsAcyclicSubgraph;

    try {
        Graph g = csvToGraph("data/silnice.csv");
        List<edge> base;
        NodeArray<int> coloring(g);

        DfsAcyclicSubgraph.call(g, base); // Line 1        

        for(List<edge>::iterator i = base.begin(); i != base.end(); i++) {
            set<edge> X; // (Indexes might be sufficient? Check later)
            X.insert(*i);
            coloring[(*i)->source()] = 0;
            coloring[(*i)->target()] = 1;
            GenCocircuits(g, X, coloring);
        }



    } catch (invalid_argument *e) {
        cerr << "Error: " << e->what() << endl;
        return 1;
    }

    return 0;
}

