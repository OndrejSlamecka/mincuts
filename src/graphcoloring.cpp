#include "graphcoloring.h"

GraphColoring::GraphColoring(const Graph &G)
{
    edges.init(G, Color::BLACK);
    vertices.init(G, Color::BLACK);
}
