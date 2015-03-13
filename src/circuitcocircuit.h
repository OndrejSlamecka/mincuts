#ifndef CIRCUITCOCIRCUIT_H
#define CIRCUITCOCIRCUIT_H

#include <stdexcept>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Queue.h>
#include <ogdf/basic/Stack.h>
#include <ogdf/basic/BinaryHeap2.h>
#include <ogdf/basic/DisjointSets.h>
#include "graphcoloring.h"

typedef struct bond {
    ogdf::List<edge> edges;
    edge lastBondFirstEdge; // c_s(j) where j is the largest index s.t. c_s(j) has been added to Y union X
} bond;

std::ostream & operator<<(std::ostream &os, const bond &S);

class CircuitCocircuit
{
    Graph &G;
    ogdf::List<Prioritized<edge, int>> allEdgesSortedByIndex;
    int cutSizeBound;

    CircuitCocircuit();

    void genStage(int components, const bond &Y, int j,
                  ogdf::List<bond> &bonds, GraphColoring &coloring,
                  const bond &X, node red, node blue);

    void shortestPath(const GraphColoring &coloring, node s,
                      const ogdf::List<edge> &forbidden, node &lastRed,
                      ogdf::List<edge> &path);

    void recolorBlack(GraphColoring &coloring, List<edge> &edges);
    void hideConnectedBlueSubgraph(const GraphColoring &coloring, node start);

    /**
     * Ignores red edges on the way!
     * @param coloring
     * @param start
     * @return
     */
    bool findPathToAnyBlueAndColorItBlue(GraphColoring &coloring, node start);
    bool reconnectBlueSubgraph(const ogdf::List<edge> &XY, const ogdf::List<edge> &X,
                               GraphColoring &coloring, node u, edge c);

    void minimalSpanningForest(int components, const bond &Y, ogdf::List<edge> &edges);

public:    
    CircuitCocircuit(Graph &Graph, int cutSizeBound) : G(Graph), cutSizeBound(cutSizeBound)
    {
        for (edge e = G.firstEdge(); e; e = e->succ()) {
            allEdgesSortedByIndex.pushBack(Prioritized<edge,int>(e, e->index()));
        }
        allEdgesSortedByIndex.quicksort();
    }

    /**
     * Runs the CircuitCocircuit algorithm, stores k-way bonds
     * @param components
     * @param bonds
     */
    void run(int components, ogdf::List<bond> &bonds);

    /**
     * Extends j-1 bond Y (possibly empty) to j-bond (which is added to bonds)
     * @param components
     * @param Y
     * @param j
     * @param bonds
     */
    void extendBond(int components, const bond &Y, int j,
                    ogdf::List<bond> &bonds);

    ~CircuitCocircuit();
};

#endif // CIRCUITCOCIRCUIT_H
