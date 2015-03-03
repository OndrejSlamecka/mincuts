#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <sstream>
#include <ogdf/basic/Graph.h>
#include "graphcoloring.h"

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

std::ostream & operator<<(std::ostream &os, const List<edge>& L)
{
    int i = 0, ls = L.size();
    for(auto e : L) {
        os << e->index() << "(" << e->source() << "," << e->target() << ")";
        if (i < ls - 1) os << ","; // This doesn't work if I have to same edges in a list,... TODO
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

void printColoring(const Graph &G, const GraphColoring &c) {
    node n;

    cout << "Nodes: ";
    forall_nodes(n, G) {
        cout << n->index() << " is " << nameColor(c[n]) << "; ";
    }
    cout << endl;

    edge e;

    cout << "Edges: ";
    forall_edges(e, G) {
        cout << e->index() << " is " << nameColor(c[e]) << "; ";
    }
    cout << endl;
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
 * Helper function todetermine whether given set of edges really is a minimal cut
 */
bool isMinCut(Graph &G, const List<edge> &cut) {
    // try to subtract each edge from the cut and test the rest with isCut

    for(auto e : cut) {
        List<edge> smallerCut = cut;
        ListIterator<edge> it = smallerCut.search(e);
        smallerCut.del(it);
        if(!isCut(G, smallerCut))
            return false;
    }

    return true;
}



#endif // HELPERS_HPP
