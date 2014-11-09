#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <ogdf/basic/Graph.h>
#include <sstream>
#include "graphcoloring.h"

using namespace ogdf;
using namespace std;

std::ostream & operator<<(std::ostream &os, const List<edge>& L)
{
    for(List<edge>::const_iterator i = L.begin(); i != L.end(); i++) {
        os << (*i)->index() << "(" << (*i)->source() << "," << (*i)->target() << ")";
        if (*i != L.back()) os << ",";
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


#endif // HELPERS_HPP
