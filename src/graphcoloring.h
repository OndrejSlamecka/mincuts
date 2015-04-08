#ifndef COLOREDGRAPH_H
#define COLOREDGRAPH_H

#include <ogdf/basic/Graph.h>

enum class Color {
    BLACK,
    RED,
    BLUE
};

class GraphColoring
{
    // TODO: Check whether map wouldn't be a better choice. (As these arrays might consume a lot of memory for uncolored vertices/edges)
    // TODO: Maybe vertices are enough?
    ogdf::NodeArray<Color> vertices;

    GraphColoring();
public:
    ogdf::EdgeArray<Color> edges; // TODO: Move to private
    ogdf::List<ogdf::node> redNodes;
    int nBlueVertices;

    GraphColoring(const ogdf::Graph &G)
    {
        edges.init(G, Color::BLACK);
        vertices.init(G, Color::BLACK);
        nBlueVertices = 0;
    }

    void printColoring();

    void set(ogdf::node v, Color c)
    {
        if (c == Color::BLUE && vertices[v] != Color::BLUE) {
            nBlueVertices++;
        }
        if (c != Color::BLUE && vertices[v] == Color::BLUE) {
            nBlueVertices--;
        }

        vertices[v] = c;
    }

    Color& operator[](ogdf::edge e) { return edges[e]; }
    Color& operator[](ogdf::node v) { return vertices[v]; }

    const Color& operator[](ogdf::edge e) const { return edges[e]; }
    const Color& operator[](ogdf::node v) const { return vertices[v]; }
};

#endif // COLOREDGRAPH_H
