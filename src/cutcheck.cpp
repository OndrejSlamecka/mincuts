#include <iostream>
#include <sstream>

#include <ogdf/basic/Graph.h>
#include "helpers.hpp"

using namespace std;

void indiciesStr2list(char *str, List<int> &l) {
	stringstream ss(str);
	string item;
	while (getline(ss, item, ',')) {
		l.pushBack(stoi(item));
	}
}

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

void printUsage(char *name)
{
	cout << "Usage:\t" << name << " <edge_file.csv> " \
		 << "[-imc, --ismincut <list>] [-cc <list>]\n\t[-rcc <results file>]" \
		 << endl;

	cout << endl \
		<< "\t-imc -- verifies if <list> is cut and minimal one" << endl \
		<< "\t-cc  -- computes # of components of G\\<list>" << endl \
		<< "\t-rcc -- randomized cuts check." << endl;

	cout << endl << "*list* is comma separated list of edge indicies." << endl;
}

int main(int argc, char *argv[])
{
    // User requested help right away
	if (argc <= 1 || argv[1] == string("-h") || argv[1] == string("--help")) {
		printUsage(argv[0]);
		exit(1);
	}

	// First argument is always file with edges (except for help)
	ifstream fEdges(argv[1]);
	if (!fEdges.is_open()) {
		cerr << "Edges file doesn't exist or could not be accessed." << endl;
		exit(2);
	}

	Graph G;
	csvToGraph(G, fEdges);

	// Determine action
	int action = 0;
	List<int> indicies;

	if (argv[2] == string("-imc") || argv[2] == string("--ismincut")) {
		action = 1;
	}

	if (argv[2] == string("-cc")) {
		action = 2;
	}

	// Perform the action
	List<edge> edges;
	if (1 <= action && action <= 2) {
		indiciesStr2list(argv[3], indicies);
		indicies2edges(G, indicies, edges);
	}

	switch (action) {
		case 1: visualize_isMinCut(G, edges);
				break;
		case 2: visualize_countComponents(G, edges);
				break;
		default: printUsage(argv[0]);
				 exit(1);
	}

	return 0;
}
