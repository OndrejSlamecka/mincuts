#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <ogdf/basic/Graph.h>
#include <sstream>

using namespace ogdf;
using namespace std;

string nameColor(int c) {
    switch(c) {
        case BLACK: return "black";
        case RED: return "red";
        case BLUE: return "blue";
        default: return "ERROR";
    }
}

std::ostream & operator<<(std::ostream &os, const List<edge>& L)
{
    for(List<edge>::const_iterator i = L.begin(); i != L.end(); i++) {
        os << (*i)->index() << "(" << (*i)->source() << "," << (*i)->target() << ")";
        if (*i != L.back()) os << ", ";
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

std::ostream & operator<<(std::ostream &os, const NodeArray<int> coloring) {
    node u;
    forall_nodes(u, *coloring.graphOf()) {
        os << u << " is " << nameColor(coloring[u]) << "; ";
    }

    return os;
}


#endif // HELPERS_HPP
