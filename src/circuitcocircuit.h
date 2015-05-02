/*
 * Copyright (c) 2015, Ondrej Slamecka <ondrej@slamecka.cz>
 * See the LICENSE file in the root folder of this repository.
 */

#ifndef SRC_CIRCUITCOCIRCUIT_H_
#define SRC_CIRCUITCOCIRCUIT_H_

#include <stdint.h>
#include <stdexcept>
#include <random>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Queue.h>
#include <ogdf/basic/Stack.h>
#include <ogdf/basic/DisjointSets.h>
#include "./graphcolouring.h"

typedef struct bond {
    ogdf::List<ogdf::edge> edges;

    /** c_s(j) where j is the largest index s.t. c_s(j) has been added
     *  to the bond */
    ogdf::edge lastBondFirstEdge;
} bond;

std::ostream & operator<<(std::ostream &os, const bond &S);

class CircuitCocircuit {
    ogdf::Graph &G;
    int cutSizeBound;

    ogdf::EdgeArray<u_int64_t> lambda;
    ogdf::List<ogdf::Prioritized<ogdf::edge, int>> allEdgesSortedByIndex;

#ifdef MEASURE_RUNTIME
    int measurementDepth;
    int nBondsOutput = 0;
#endif

    /** Output to stdout or store into list "bonds" given to the run method? */
    bool outputToStdout = false;

    CircuitCocircuit();

    void extendBond(int components, const bond &Y, int j,
                    ogdf::List<bond> &bonds);

    void genStage(GraphColouring &colouring, int components, const bond &Y,
                  int j, ogdf::List<bond> &stageBonds, const bond &X);

    ogdf::node getStartNodeOfIotaMinimalPath(GraphColouring &colouring,
                                             ogdf::NodeArray<ogdf::edge> &accessEdge,
                                             ogdf::node s1, ogdf::node s2);
    void shortestPath(GraphColouring &colouring, const ogdf::List<ogdf::edge> &Y,
                      const ogdf::List<ogdf::edge> &X, ogdf::node &lastRed,
                      ogdf::List<ogdf::edge> &path);

    void revertColouring(GraphColouring &colouring, ogdf::List<ogdf::edge> &P,
                        ogdf::List<ogdf::edge> &blueEdges,
                        ogdf::node firstRed, const bond &X,
                        ogdf::List<ogdf::edge> &oldBlueTreeEdges,
                        ogdf::List<ogdf::edge> &newBlueTreeEdges);

    bool isBlueTreeDisconnected(GraphColouring &colouring, ogdf::edge c,
                                ogdf::node u);

    void recolourBlueTreeBlack(GraphColouring &colouring, ogdf::node start,
                               ogdf::List<ogdf::edge> &oldBlueTreeEdges);

    bool reCreateBlueTreeIfDisconnected(GraphColouring &colouring,
                                        const ogdf::List<ogdf::edge> &Y,
                                        const ogdf::List<ogdf::edge> &X,
                                        ogdf::node v, ogdf::edge c,
                                        ogdf::List<ogdf::edge> &oldBlueTreeEdges,
                                        ogdf::List<ogdf::edge> &newBlueTreeEdges);

    void minimalSpanningForest(int components, const bond &Y,
                               ogdf::List<ogdf::edge> &edges);

 public:
#ifdef MEASURE_RUNTIME
    CircuitCocircuit(ogdf::Graph &Graph, int cutSizeBound, int measurementDepth);
#else
    CircuitCocircuit(ogdf::Graph &Graph, int cutSizeBound);
#endif

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

#endif  // SRC_CIRCUITCOCIRCUIT_H_
