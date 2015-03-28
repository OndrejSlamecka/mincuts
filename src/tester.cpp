#include <iostream>
#include <stdexcept>
#include <set>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/graph_generators.h>

#include "helpers.h"
#include "circuitcocircuit.h"

using namespace ogdf;
using namespace std;

/**
 * OGDF's solution is probably biased... TODO
 * http://stackoverflow.com/a/14618505/247532
 */
void randomConnectedGraph(Graph &G, int nodes, int edges)
{
	randomSimpleGraph(G, nodes, edges);
	List<edge> added;
	makeConnected(G, added);
}

ostream & operator<<(std::ostream &os, const set<int>& S)
{
	int i = 0, ss = S.size();
	for (auto e : S) {
		os << e;
		if (i < ss - 1) os << ",";
		i++;
	}
	return os;
}

void printUsage(char *name)
{
	cout << "Usage:\t" << name << " <# of nodes> <min>-<max edges> " \
		 << "<cut size bound> <# of components>\n" << endl;

	cout << "\tGenerates random graphs to test CircuitCocircuit " \
	     << "implementation\n" << endl;
	cout << "\t<# of components> can be exact or range (e.g. 2-3)" << endl;
	cout << "\t<max edges> is not strict (if the random graph is " \
		 << "disconnected\n\t\tthen we add edges to connect it" << endl;
}

int main(int argc, char *argv[])
{
	if (argc != 5) {
		printUsage(argv[0]);
		exit(1);
	}

	int nodes, minEdges, maxEdges,
		cutSizeBound, minComponents, maxComponents;
	try {
		size_t hyphenPos;

		// # of nodes
		nodes = stoi(argv[1]);

		// min-max edges
		string secondArg(argv[2]);
		hyphenPos = secondArg.find('-');
		if (hyphenPos == string::npos) {
			throw invalid_argument("Number of edges is not range.");
		}
		minEdges = stoi(secondArg.substr(0, hyphenPos));
		maxEdges = stoi(secondArg.substr(hyphenPos + 1));

		if (maxEdges < minEdges) {
			throw invalid_argument("max edges < min edges");
		}

		// cut size bound
		cutSizeBound = stoi(argv[3]);

		// # of components
		string fourthArg(argv[4]);
		hyphenPos = fourthArg.find('-');
		if (hyphenPos == string::npos) {
			minComponents = maxComponents = stoi(argv[4]);
		} else {
			minComponents = stoi(fourthArg.substr(0, hyphenPos));
			maxComponents = stoi(fourthArg.substr(hyphenPos + 1));
		}

		if (maxComponents < minComponents) {
			throw invalid_argument("max components < min components");
		}
	} catch(invalid_argument &_) { // stoi failed
		printUsage(argv[0]);
		exit(2);
	}

	int edges;
	for(;;) {
		Graph G;

		edges = randomNumber(minEdges, maxEdges);
		randomConnectedGraph(G, nodes, edges);

		// run circuitcocircuit and bfc
		List<bond> bonds;
		CircuitCocircuit alg(G, cutSizeBound);
		for (int i = minComponents; i <= maxComponents; ++i) {
			alg.run(i, bonds);
		}

		List<List<edge>> bf_bonds;
		bruteforceGraphBonds(G, cutSizeBound, minComponents, maxComponents,
				bf_bonds);

		set<set<int>> A, B, AmB, BmA;
		for(bond b : bonds) {
			set<int> si;
			for (edge e : b.edges) si.insert(e->index());
			A.insert(si);
		}

		for(List<edge> le : bf_bonds) {
			set<int> si;
			for (edge e : le) si.insert(e->index());
			B.insert(si);
		}

		set_difference(A.begin(), A.end(), B.begin(), B.end(),
			inserter(AmB, AmB.end()));
		set_difference(B.begin(), B.end(), A.begin(), A.end(),
			inserter(BmA, BmA.end()));

		if (AmB.empty() && BmA.empty()) {
			cout << ".";
			cout.flush();
		} else {
			cout << endl;
			cout << "mincuts / bf:" << endl;
			for (set<int> c : AmB) {
				cout << c << endl;
			}

			cout << "bf / mincuts:" << endl;
			for (set<int> c : BmA) {
				cout << c << endl;
			}

			ofstream fGraph("tmp/tester_in.csv");
			if (!fGraph.is_open()) {
				cerr << "tmp/tester_in.csv could not be opened" << endl;
				exit(3);
			}
			graph2csv(G, fGraph);
			cout << "Input graph written to tmp/tester_in.csv" << endl;

			break;
		}
	}

	return 0;
}
