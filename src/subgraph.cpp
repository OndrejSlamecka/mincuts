#include "subgraph.h"

Subgraph::Subgraph(const Graph &G)
{
    edges.init(G, false);
    vertices.init(G, false);
}
