#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <sstream>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>
#include "graphcoloring.h"

#include <set>

/**
 * Reads a csv file with lines "<id>;<source>;<target>;..." and transforms it into a graph
 */
void csvToGraph(Graph &G, ifstream &fEdges);

std::ostream & operator<<(std::ostream &os, const std::set<edge> &S);
std::ostream & operator<<(std::ostream &os, const ogdf::List<edge> &L);
std::ostream & operator<<(std::ostream &os, const ogdf::Graph &G);

string nameColor(Color c);

string coloring2str(const ogdf::Graph &G, const GraphColoring &c);

ogdf::edge edgeByIndex(const ogdf::List<edge> &edges, int index);
std::string edgeInfo(const ogdf::Graph &G, int index);

void indicies2edges(const ogdf::List<edge> &graphEdges, const string &str, ogdf::List<edge> &l);

/**
 * Helper function to determine whether given set of edges really is a cut
 */
bool isCut(Graph &G, const ogdf::List<edge> &cut);

/**
 * Helper function todetermine whether given set of edges really is a minimal cut, w.r.t. # of components of G\cut
 * Returns 0 on success, -1 if # of components is 1. Otherwise, the # of components of G\cut
 */
int isMinCut(Graph &G, const ogdf::List<edge> &cut, int &ncomponents);

int isMinCut(Graph &G, const ogdf::List<edge> &cut);


#endif // HELPERS_HPP
