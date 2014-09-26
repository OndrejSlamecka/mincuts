#ifndef SUBGRAPH_H
#define SUBGRAPH_H

#include <ogdf/basic/Graph.h>

using namespace ogdf;

class Subgraph
{
    NodeArray<bool> vertices;
    EdgeArray<bool> edges;

    Subgraph();
public:
    Subgraph(const Graph &G);

    bool& operator[](edge e) { return edges[e]; }
    bool& operator[](node v) { return vertices[v]; }

    const bool& operator[](edge e) const { return edges[e]; }
    const bool& operator[](node v) const { return vertices[v]; }
};

#endif // SUBGRAPH_H
