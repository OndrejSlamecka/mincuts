#ifndef CIRCUITCOCIRCUIT_H
#define CIRCUITCOCIRCUIT_H

#include <stdexcept>
#include <random>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Queue.h>
#include <ogdf/basic/Stack.h>
#include <ogdf/basic/DisjointSets.h>
#include "graphcoloring.h"

typedef struct bond {
    ogdf::List<ogdf::edge> edges;
    ogdf::edge lastBondFirstEdge; // c_s(j) where j is the largest index s.t. c_s(j) has been added to bond
} bond;

std::ostream & operator<<(std::ostream &os, const bond &S);

class CircuitCocircuit
{
    ogdf::Graph &G;
    int cutSizeBound;

    ogdf::EdgeArray<int> lambda;
    ogdf::List<ogdf::Prioritized<ogdf::edge, int>> allEdgesSortedByIndex;

    bool outputToStdout = false; // Output to stdout or store into list "bonds" given to the run method?

    CircuitCocircuit();

    /**
     * Extends (possibly empty) j-1 bond Y to j-bond (which is added to bonds)
     * @param components
     * @param Y
     * @param j
     * @param bonds
     */
    void extendBond(int components, const bond &Y, int j, ogdf::List<bond> &bonds);

    void genStage(GraphColoring &coloring, int components, const bond &Y, int j,
                  ogdf::List<bond> &stageBonds, const bond &X);

    ogdf::node lexicographicallyMinimalPathStartNode(GraphColoring &coloring,
                                                     ogdf::NodeArray<ogdf::edge> &accessEdge,
                                                     ogdf::node s1, ogdf::node s2);
    void shortestPath(GraphColoring &coloring, const ogdf::List<ogdf::edge> &Y,
                      const ogdf::List<ogdf::edge> &X, ogdf::node &lastRed,
                      ogdf::List<ogdf::edge> &path);

    void revertColoring(GraphColoring &coloring, ogdf::List<ogdf::edge> &P,
                        ogdf::List<ogdf::edge> &blueEdges,
                        ogdf::node firstRed, const bond &X,
                        ogdf::List<ogdf::edge> &oldBlueTreeEdges,
                        ogdf::List<ogdf::edge> &newBlueTreeEdges);

    bool isBlueTreeDisconnected(GraphColoring &coloring, ogdf::edge c, ogdf::node u);

    void recolorBlueTreeBlack(GraphColoring &coloring, ogdf::node start, ogdf::List<ogdf::edge> &oldBlueTreeEdges);

    bool recreateBlueTreeIfDisconnected(GraphColoring &coloring,
                                        const ogdf::List<ogdf::edge> &Y,
                                        const ogdf::List<ogdf::edge> &X,
                                        ogdf::node v, ogdf::edge c,
                                        ogdf::List<ogdf::edge> &oldBlueTreeEdges,
                                        ogdf::List<ogdf::edge> &newBlueTreeEdges);

    void minimalSpanningForest(int components, const bond &Y, ogdf::List<ogdf::edge> &edges);

public:    
    CircuitCocircuit(ogdf::Graph &Graph, int cutSizeBound) : G(Graph), cutSizeBound(cutSizeBound), lambda(G)
    {
        // Sort edges by index for use by minimalSpanningForest
        for (ogdf::edge e = G.firstEdge(); e; e = e->succ()) {
            allEdgesSortedByIndex.pushBack(ogdf::Prioritized<ogdf::edge,int>(e, e->index()));
        }
        allEdgesSortedByIndex.quicksort();

        // Create map lambda : E(G) -> N (natural numbers) for selection of shortest path
        // The map is randomized with each algorithm run in order to detect mistakes
        // related to graph traversing order
        std::default_random_engine generator;
        int upper_bound = G.numberOfNodes() * G.numberOfNodes() + 10; // + 10 for small graphs
        std::uniform_int_distribution<int> distribution(1, upper_bound);

        ogdf::edge e;
        forall_edges(e, G) {
            lambda[e] = distribution(generator);
        }
    }

    /**
     * Runs the CircuitCocircuit algorithm, stores k-bonds
     * If you expect a lot of bonds then your "bonds" list might run out of
     * memory. Use the other run method which outputs them to stdout directly
     *
     * @param k     The number of components you want your bonds to have
     * @param bonds List to store the bonds
     */
    void run(int k, ogdf::List<bond> &bonds);

    /**
     * Runs the CircuitCocircuit algorithm, prints k-bonds to stdout
     *
     * @param k     The number of components you want your bonds to have
     */
    void run(int components);

    ~CircuitCocircuit();
};

#endif // CIRCUITCOCIRCUIT_H
