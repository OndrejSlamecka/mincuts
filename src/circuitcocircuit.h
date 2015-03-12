#ifndef CIRCUITCOCIRCUIT_H
#define CIRCUITCOCIRCUIT_H

#include <stdexcept>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Queue.h>
#include <ogdf/basic/Stack.h>
#include <ogdf/basic/BinaryHeap2.h>
#include <ogdf/basic/DisjointSets.h>
#include "graphcoloring.h"

class CircuitCocircuit
{
    Graph &G;
    ogdf::List<Prioritized<edge, int>> allEdgesSortedByIndex;
    int cutSizeBound;

    CircuitCocircuit();

    void genStage(int components, const ogdf::List<edge> &Y, int j,
                  ogdf::List<ogdf::List<edge>> &bonds, GraphColoring &coloring,
                  const ogdf::List<edge> &X, node red, node blue);

    void shortestPath(const GraphColoring &coloring, node s,
                      const ogdf::List<edge> &forbidden, node &lastRed,
                      ogdf::List<edge> &path);

    void recolorBlack(GraphColoring &coloring, List<edge> &edges);
    void hideConnectedBlueSubgraph(const GraphColoring &coloring, node start);

    /**
     * @brief findPathToAnyBlueAndColorItBlue Ignores red edges on the way!
     * @param coloring
     * @param start
     * @return
     */
    bool findPathToAnyBlueAndColorItBlue(GraphColoring &coloring, node start);
    bool reconnectBlueSubgraph(const ogdf::List<edge> &XY, const ogdf::List<edge> &X,
                               GraphColoring &coloring, node u, edge c);

    void minimalSpanningForest(int components, const ogdf::List<edge> &Y, ogdf::List<edge> &edges);

public:    
    CircuitCocircuit(Graph &Graph, int cutSizeBound) : G(Graph), cutSizeBound(cutSizeBound)
    {
        for (edge e = G.firstEdge(); e; e = e->succ()) {
            allEdgesSortedByIndex.pushBack(Prioritized<edge,int>(e, e->index()));
        }
        allEdgesSortedByIndex.quicksort();
    }
    void run(int components, ogdf::List<ogdf::List<edge>> &bonds);
    void extendBond(int components, const ogdf::List<edge> &Y, int j,
                    ogdf::List<ogdf::List<edge>> &bonds);

    ~CircuitCocircuit();
};

#endif // CIRCUITCOCIRCUIT_H
