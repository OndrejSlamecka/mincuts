#ifndef COLOREDGRAPH_H
#define COLOREDGRAPH_H

#include <ogdf/basic/Graph.h>

using namespace ogdf;

enum class Color {
    BLACK,
    RED,
    BLUE
};

class ColoredGraph : public Graph
{
    NodeArray<Color> vertices;
    EdgeArray<Color> edges;

public:
    ColoredGraph();

    Color& operator[](edge e) { return edges[e]; }
    Color& operator[](node v) { return vertices[v]; }

    const Color& operator[](edge e) const { return edges[e]; }
    const Color& operator[](node v) const { return vertices[v]; }
};

#endif // COLOREDGRAPH_H
