/*
 * Copyright (c) 2015, Ondrej Slamecka <ondrej@slamecka.cz>
 * See the LICENSE file in the root folder of this repository.
 */

#ifndef SRC_HELPERS_H_
#define SRC_HELPERS_H_

#include <string>
#include <sstream>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <set>
#include "./graphcolouring.h"

/**
 * Reads a csv file with lines "<id>;<source>;<target>;..." and transforms it
 * into a graph
 */
void csv2graph(ogdf::Graph &G, std::ifstream &fEdges);
void graph2csv(const ogdf::Graph &G, std::ostream &fGraph);
void graph2dot(const ogdf::Graph &G, std::ostream &fGraph);

std::ostream & operator<<(std::ostream &os, const std::set<int> &S);
std::ostream & operator<<(std::ostream &os, const std::set<ogdf::edge> &S);
std::ostream & operator<<(std::ostream &os, const ogdf::List<ogdf::edge> &L);

std::string nameColor(Colour c);

std::string coloring2str(const ogdf::Graph &G, const GraphColouring &c);
std::string edgelist2str(const ogdf::List<ogdf::edge> &edges);

ogdf::edge edgeByIndex(const ogdf::List<ogdf::edge> &edges, int index);
std::string edgeInfo(const ogdf::Graph &G, int index);

void indicies2edges(const ogdf::List<ogdf::edge> &graphEdges,
        const std::string &str, ogdf::List<ogdf::edge> &l);

/**
 * Helper function to determine whether given set of edges really is a cut
 */
bool isCut(ogdf::Graph &G, const ogdf::List<ogdf::edge> &cut);

/**
 * Helper function todetermine whether given set of edges really is a minimal
 * cut, w.r.t. # of components of G\cut
 * Returns 0 on success, -1 if # of components is 1. Otherwise, the # of
 * components of G\cut
 */
int isMinCut(ogdf::Graph &G, const ogdf::List<ogdf::edge> &cut,
             int &ncomponents);

int isMinCut(ogdf::Graph &G, const ogdf::List<ogdf::edge> &cut);

/**
 * Computes all bonds of maximum # of edges cutSizeBound which disconnect
 * G to at least minComponents and at max maxComponents.
 */
void bruteforceGraphBonds(ogdf::Graph &G, int cutSizeBound, int minComponents,
                int maxComponents, ogdf::List<ogdf::List<ogdf::edge>> &bonds);

#endif  // SRC_HELPERS_H_
