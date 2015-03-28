#ifndef CIRCUITCOCIRCUIT_H
#define CIRCUITCOCIRCUIT_H

#include <stdexcept>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Queue.h>
#include <ogdf/basic/Stack.h>
#include <ogdf/basic/DisjointSets.h>
#include "graphcoloring.h"

typedef struct bond {
    ogdf::List<ogdf::edge> edges;
    ogdf::edge lastBondFirstEdge; // c_s(j) where j is the largest index s.t. c_s(j) has been added to Y union X
} bond;

std::ostream & operator<<(std::ostream &os, const bond &S);

class CircuitCocircuit
{
    ogdf::Graph &G;
    ogdf::List<ogdf::Prioritized<ogdf::edge, int>> allEdgesSortedByIndex;
    int cutSizeBound;

    CircuitCocircuit();

    void genStage(int components, const bond &Y, int j,
                  ogdf::List<bond> &bonds, GraphColoring &coloring,
                  const bond &X);

    void shortestPath(const GraphColoring &coloring, const ogdf::List<ogdf::edge> &forbidden,
                      ogdf::node &lastRed, ogdf::List<ogdf::edge> &path);

    void revertColoring(GraphColoring &coloring, ogdf::List<ogdf::edge> &edges,
                        ogdf::List<ogdf::edge> &blueEdges, ogdf::node firstRed,
                        ogdf::List<ogdf::edge> &reconnectionBlues,
                        const bond &X);
    void hideConnectedBlueSubgraph(const GraphColoring &coloring, ogdf::node start);

    /**
     * Ignores red edges on the way!
     * @param coloring
     * @param start
     * @return
     */
    bool findPathToAnyBlueAndColorItBlue(GraphColoring &coloring, ogdf::node start,
                                         ogdf::List<ogdf::edge> &reconnectionBlues);
    bool isBlueSubgraphDisconnected(GraphColoring &coloring, const bond &X, ogdf::edge c, ogdf::node u);
    bool reconnectBlueSubgraph(const ogdf::List<ogdf::edge> &XY, GraphColoring &coloring, ogdf::node u, ogdf::edge c,
                               ogdf::List<ogdf::edge> &reconnectionBlues);

    void minimalSpanningForest(int components, const bond &Y, ogdf::List<ogdf::edge> &edges);

public:    
    CircuitCocircuit(ogdf::Graph &Graph, int cutSizeBound) : G(Graph), cutSizeBound(cutSizeBound)
    {
        for (ogdf::edge e = G.firstEdge(); e; e = e->succ()) {
            allEdgesSortedByIndex.pushBack(ogdf::Prioritized<ogdf::edge,int>(e, e->index()));
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
    void extendBond(int components, const bond &Y, GraphColoring &coloring,
                    int j, ogdf::List<bond> &bonds);

    ~CircuitCocircuit();
};

#endif // CIRCUITCOCIRCUIT_H
