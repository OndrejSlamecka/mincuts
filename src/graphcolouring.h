/*
 * Copyright (c) 2015, Ondrej Slamecka <ondrej@slamecka.cz>
 * See the LICENSE file in the root folder of this repository.
 */

#ifndef SRC_GRAPHCOLOURING_H_
#define SRC_GRAPHCOLOURING_H_

#include <ogdf/basic/Graph.h>

enum class Colour {
    BLACK,
    RED,
    BLUE
};

class GraphColouring {
    // TODO: Check whether map wouldn't be a better choice. (As these arrays
    // might consume a lot of memory for uncoloured vertices/edges)
    ogdf::NodeArray<Colour> vertices;
    ogdf::List<ogdf::node> redVertices;

    ogdf::EdgeArray<Colour> edges;

    GraphColouring();

 public:
    int nBlueVertices;

    explicit GraphColouring(const ogdf::Graph &G) {
        edges.init(G, Colour::BLACK);
        vertices.init(G, Colour::BLACK);
        nBlueVertices = 0;
    }

    void set(ogdf::node v, Colour c) {
        if (c == Colour::BLUE && vertices[v] != Colour::BLUE) {
            nBlueVertices++;
        }
        if (c != Colour::BLUE && vertices[v] == Colour::BLUE) {
            nBlueVertices--;
        }

        if (vertices[v] == Colour::RED && c != Colour::RED) {
            redVertices.popBack();
        }
        if (vertices[v] != Colour::RED && c == Colour::RED) {
            redVertices.pushBack(v);
        }

        vertices[v] = c;
    }

    const ogdf::List<ogdf::node>& getRedVertices() {
        return redVertices;
    }

    Colour& operator[](ogdf::edge e) {
        return edges[e];
    }

    const Colour& operator[](ogdf::edge e) const {
        return edges[e];
    }

    const Colour& operator[](ogdf::node v) const {
        return vertices[v];
    }
};

#endif  // SRC_GRAPHCOLOURING_H_
