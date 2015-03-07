#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <sstream>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>
#include "graphcoloring.h"

#include <set> // TODO: Not necessary, remove

using namespace ogdf;
using namespace std;

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

std::ostream & operator<<(std::ostream &os, const set<edge>& S){
    int i = 0, ss = S.size();
    for(auto e : S) {
        os << e->index(); // << "(" << e->source() << "," << e->target() << ")";
        if (i < ss - 1) os << ",";
        i++;
    }
    return os;
}

std::ostream & operator<<(std::ostream &os, const List<edge>& L)
{
    int i = 0, ls = L.size();
    for(auto e : L) {
        os << e->index(); //<< "(" << e->source() << "," << e->target() << ")";
        if (i < ls - 1) os << ",";
        i++;
    }
    return os;
}


std::ostream & operator<<(std::ostream &os, const Graph& G)
{
    edge e;
    forall_edges(e, G) {
        os << "(" << e->source() << "," << e->target() << "), ";
    }
    return os;
}

string nameColor(Color c) {
    switch(c) {
        case Color::BLACK: return "black";
        case Color::RED: return "red";
        case Color::BLUE: return "blue";
        default: return "ERROR";
    }
}

string coloring2str(const Graph &G, const GraphColoring &c) {
    stringstream s;

    node n;

    s << "Nodes: ";
    forall_nodes(n, G) {
        s << n->index() << " is " << nameColor(c[n]) << "; ";
    }
    s << endl;

    edge e;

    s << "Edges: ";
    forall_edges(e, G) {
        s << e->index() << " is " << nameColor(c[e]) << "; ";
    }
    s << endl;

    return s.str();
}

/*void printColoring(Graph &G)
{
    node u;
    forall_nodes(u, G) {
        cout << u << " is " << nameColor(G[u]) << "; ";
    }
}*/

edge edgeByIndex(const List<edge> &edges, int index) {
    for(auto it = edges.begin(); it != edges.end(); ++it) if((*it)->index() == index) return *it;
    return nullptr;
}

void indicies2edges(const Graph &G, List<int> indicies, List<edge> &edges) {
    List<edge> allEdges;
    G.allEdges(allEdges);
    for(int i : indicies) {
        edge e;
        for (edge f : allEdges) {
            if (f->index() == i)
                e = f;
        }

        edges.pushBack(e);
    }
}


/**
 * Helper function to determine whether given set of edges really is a cut
 */
bool isCut(Graph &G, const List<edge> &cut) {
    for (auto e : cut) {
        G.hideEdge(e);
    }

    bool r = isConnected(G);

    G.restoreAllEdges();
    return !r;
}

/**
 * Helper function todetermine whether given set of edges really is a minimal cut, w.r.t. # of components of G\cut
 * Returns 0 on success, -1 if # of components is 1. Otherwise, the # of components of G\cut
 */
int isMinCut(Graph &G, const List<edge> &cut) {
    // try to subtract each edge from the cut and test the rest with isCut

    NodeArray<int> component(G);

    for(auto e : cut) {
        G.hideEdge(e);
    }
    int ncomponents = connectedComponents(G, component);

	if (ncomponents == 1) return -1;

    for(auto e : cut) {
        G.restoreEdge(e);

        // Count # of components of smaller cut
        int nSmallerCutComponents = connectedComponents(G, component);

        // If numbers of components is the same and it's still a cut then obviously the original cut was not minimal
        if(nSmallerCutComponents == ncomponents) {
            G.hideEdge(e);
            return ncomponents;
        }
        G.hideEdge(e);
    }

    return 0;
}



#endif // HELPERS_HPP
