/*
 * Copyright (c) 2015, Ondrej Slamecka <ondrej@slamecka.cz>
 * See the LICENSE file in the root folder of this repository.
 */

#ifndef SRC_CIRCUITCOCIRCUIT_H_
#define SRC_CIRCUITCOCIRCUIT_H_

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Queue.h>
#include <ogdf/basic/Stack.h>
#include <ogdf/basic/DisjointSets.h>
#include <stdint.h>
#include <stdexcept>
#include <random>
#include "./dyncon/dyn_con.h"
#include "./graphcolouring.h"

typedef struct bond {
    ogdf::List<ogdf::edge> edges;

    /** c_s(j) where j is the largest index s.t. c_s(j) has been added
     *  to the bond */
    ogdf::edge lastBondFirstEdge;
} bond;

std::ostream & operator<<(std::ostream &os, const bond &S);

class CircuitCocircuit {
    DCGraph &G;
    // Red edges and X are removed from dyncon, the rest stays
    dyn_con dyncon;
    int cutSizeBound;

    ogdf::EdgeArray<u_int64_t> lambda;
    ogdf::List<ogdf::Prioritized<ogdf::edge, int>> allEdgesSortedByIndex;

    // Helper for debugging TODO: remove
    ogdf::Graph T;
    ogdf::Graph::HiddenEdgeSet hiddenInT;
    ogdf::EdgeArray<ogdf::edge> GtoT;
    ogdf::NodeArray<ogdf::node> GtoTnodes;

#ifdef MEASURE_RUNTIME
    int measurementDepth;
    int nBondsOutput = 0;
#endif

    /** Output to stdout or store into list "bonds" given to the run method? */
    bool outputToStdout = false;

    CircuitCocircuit();

    void extendBond(int components, const bond &Y, int j,
                    ogdf::List<bond> &bonds, ogdf::List<ogdf::edge> &redEdges);

    void genStage(GraphColouring &colouring, int components, const bond &Y,
                  int j, ogdf::List<bond> &stageBonds, const bond &X,
                  ogdf::List<ogdf::edge> &redEdges);

    ogdf::node getStartNodeOfIotaMinimalPath(GraphColouring &colouring,
                                             ogdf::NodeArray<ogdf::edge> &accessEdge,
                                             ogdf::node s1, ogdf::node s2);
    void shortestPath(GraphColouring &colouring, const ogdf::List<ogdf::edge> &Y,
                      const ogdf::List<ogdf::edge> &X, ogdf::node &lastRed,
                      ogdf::List<ogdf::edge> &path);

    void revertColouring(GraphColouring &colouring, ogdf::List<ogdf::edge> &P,
                        ogdf::node firstRed, const bond &X);

    void minimalSpanningForest(int components, const bond &Y,
                               ogdf::List<ogdf::edge> &edges);

 public:
#ifdef MEASURE_RUNTIME
    CircuitCocircuit(DCGraph &Graph, int cutSizeBound, int measurementDepth);
#else
    CircuitCocircuit(DCGraph &Graph, int cutSizeBound);
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
