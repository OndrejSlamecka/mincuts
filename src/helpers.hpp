#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <ogdf/basic/Graph.h>
#include <sstream>
#include "coloredgraph.h"

using namespace ogdf;
using namespace std;

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

string nameColor(Color c) {
    switch(c) {
    case Color::BLACK: return "black";
    case Color::RED: return "red";
    case Color::BLUE: return "blue";
    default: return "ERROR";
    }
}

void printColoring(ColoredGraph &G)
{
    node u;
    forall_nodes(u, G) {
        cout << u << " is " << nameColor(G[u]) << "; ";
    }
}


int cptoi(char *cp) {
    std::istringstream iss(cp);
    int val;

    if (iss >> val) {
        return val;
    }

    throw new invalid_argument("Invalid argument supplied to cptoi - not a number.");
}


#endif // HELPERS_HPP
