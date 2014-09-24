#ifndef COLOREDGRAPH_H
#define COLOREDGRAPH_H

#include <ogdf/basic/Graph.h>

using namespace ogdf;

enum class Color {
    BLACK,
    RED,
    BLUE
};

class GraphColoring
{
    NodeArray<Color> vertices;
    EdgeArray<Color> edges;

    GraphColoring();
public:
    GraphColoring(const Graph &G);

    void printColoring();

    Color& operator[](edge e) { return edges[e]; }
    Color& operator[](node v) { return vertices[v]; }

    const Color& operator[](edge e) const { return edges[e]; }
    const Color& operator[](node v) const { return vertices[v]; }
};

#endif // COLOREDGRAPH_H
