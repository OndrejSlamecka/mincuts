#include "coloredgraph.h"

ColoredGraph::ColoredGraph()
{
    edges.init(*this, Color::BLACK);
    vertices.init(*this, Color::BLACK);
}
