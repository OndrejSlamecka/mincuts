/*
 * Copyright (c) 2015, Ondrej Slamecka <ondrej@slamecka.cz>
 * See the LICENSE file in the root folder of this repository.
 */

#include <iostream>
#include <sstream>

#include <ogdf/basic/Graph.h>
#include "helpers.h"

using namespace std;
using namespace ogdf;

void visualize_isMinCut(Graph &G, const List<edge> &edges)
{
	int n = isMinCut(G, edges);
	if (n == 0) {
		cout << "true" << endl;
	} else if (n == -1) {
		cout << "false_not_cut " << endl;
	} else {
		cout << "false_not_minimal" << endl;
	}
}

void visualize_countComponents(Graph &G, const List<edge> &edges)
{
	NodeArray<int> component(G);

	for (auto e : edges) {
	    G.hideEdge(e);
	}
	int ncomponents = connectedComponents(G, component);
	G.restoreAllEdges();

	cout << ncomponents << endl;
}

void visualize_totalCutsCheck(Graph &G, const List<List<edge>> &cuts,
		bool force)
{
	int fails = 0;
	for(List<edge> cut : cuts) {
		int n = isMinCut(G, cut);
		if (n == 0) {
			cout << ".";
		} else {
			cout << "FAIL on cut: " << cut << endl;
			++fails;
			if (!force) {
				break;
			}
		}
	}

	cout << endl << "DONE, fails: " << fails << "/" << cuts.size() << endl \
		 << "(-tcc stops on first failure, use -tccf to find all) " << endl;
}

void visualize_randomizedCutsCheck(Graph &G, const List<List<edge>> &cuts)
{
	int c = 5000;

	List<List<edge>> test;
	int ncuts = cuts.size();

	if (ncuts < c) {
		visualize_totalCutsCheck(G, cuts, true);
	} else {
		int i = 0;
		for(List<edge> cut : cuts) {
			if (i % (ncuts / c) == 0) {
				test.pushBack(cut);
			}
			++i;
		}

		visualize_totalCutsCheck(G, test, true);
	}
}

void printUsage(char *name)
{
	cerr << "Usage:\t" << name << " <edge_file.csv> " \
		 << "[-imc, --ismincut <list>] [-cc <list>]\n\t[-rcc <cuts file>] " \
		 << "[-tcc[f] <cuts file>]" << endl;

	cerr << endl \
		<< "\t-imc    -- verifies whether <list> is a cut and a minimal one" << endl \
		<< "\t-cc     -- computes the number of components of G\\<list>" << endl \
		<< "\t-rcc    -- randomized cut checker" << endl \
		<< "\t-tcc[f] -- total cut checker, 'f' counts all failures" << endl;

	cerr << endl << "*list* is a comma separated list of edge indicies." << endl;
}

int main(int argc, char *argv[])
{
	ios_base::sync_with_stdio(false);

    // User requested help right away
	if (argc != 4 || argv[1] == string("-h") || argv[1] == string("--help")) {
		printUsage(argv[0]);
		exit(1);
	}

	// First argument is always file with edges (except for help)
	Graph G;
	ifstream fGraph(argv[1]);
	if (!fGraph.is_open()) {
		cerr << "Graph file " << argv[1] << "doesn't exist or could not be " \
				"accessed. Terminating." << endl;
		exit(3);
	}
	csv2graph(G, fGraph);

	// Determine action
	int action = 0;
	bool tccForce = false;

	if (argv[2] == string("-imc") || argv[2] == string("--ismincut")) {
		action = 1;
	}

	if (argv[2] == string("-cc")) {
		action = 2;
	}

	if (argv[2] == string("-rcc")) {
		action = 3;
	}

	if (argv[2] == string("-tcc")) {
		action = 4;
	}

	if (argv[2] == string("-tccf")) {
		action = 4;
		tccForce = true;
	}

	// Perform the action
	List<edge> edges;
	List<edge> allEdges;
	G.allEdges(allEdges);

	if (1 <= action && action <= 2) {
		indicies2edges(allEdges, string(argv[3]), edges);
	}

	List<List<edge>> cuts;
	if (3 <= action && action <= 4) {
		ifstream fCuts(argv[3]);
		if (!fCuts.is_open()) {
			cerr << "Cuts file doesn't exist or could not be accessed." \
				 << endl;
			exit(2);
		}

		for (string line; getline(fCuts, line);) {
			List<edge> cut;
			indicies2edges(allEdges, line, cut);
			cuts.pushBack(cut);
		}
	}

	switch (action) {
		case 1:
			visualize_isMinCut(G, edges);
			break;
		case 2:
			visualize_countComponents(G, edges);
			break;
		case 3:
			visualize_randomizedCutsCheck(G, cuts);
			break;
		case 4:
			visualize_totalCutsCheck(G, cuts, tccForce);
			break;
		default:
		   	printUsage(argv[0]);
			exit(1);
	}

	return 0;
}
