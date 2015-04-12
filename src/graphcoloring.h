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
    ogdf::NodeArray<Color> vertices;
    ogdf::List<ogdf::node> redVertices;

    ogdf::EdgeArray<Color> edges;

    GraphColoring();

public:    
    int nBlueVertices;

    GraphColoring(const ogdf::Graph &G)
    {
        edges.init(G, Color::BLACK);
        vertices.init(G, Color::BLACK);
        nBlueVertices = 0;
    } 

    // TODO: Measure the speed difference when setBlue, setRed, setBlack are separated
    void set(ogdf::node v, Color c)
    {
        if (c == Color::BLUE && vertices[v] != Color::BLUE) {
            nBlueVertices++;
        }
        if (c != Color::BLUE && vertices[v] == Color::BLUE) {
            nBlueVertices--;
        }

        if (vertices[v] == Color::RED && c != Color::RED) {
            redVertices.popBack();
        }
        if (vertices[v] != Color::RED && c == Color::RED) {
            redVertices.pushBack(v);
        }

        vertices[v] = c;
    }

    const ogdf::List<ogdf::node>& getRedVertices() {
        return redVertices;
    }

    Color& operator[](ogdf::edge e) { return edges[e]; }

    const Color& operator[](ogdf::edge e) const { return edges[e]; }
    const Color& operator[](ogdf::node v) const { return vertices[v]; }
};

#endif // COLOREDGRAPH_H
