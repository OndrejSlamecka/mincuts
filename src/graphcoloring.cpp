#include "graphcoloring.h"

GraphColoring::GraphColoring(const ogdf::Graph &G)
{
    edges.init(G, Color::BLACK);
    vertices.init(G, Color::BLACK);
}
