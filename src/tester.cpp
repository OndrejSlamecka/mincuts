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

int main(/*int argc, char *argv[]*/)
{
	// TODO: Help
	int cutSizeBound = 4,
		minComponents = 2,
		maxComponents = 2,
		nodes = 15,
		minEdges = 20,
		maxEdges = 50;
/*	int cutSizeBound = 3,
		minComponents = 2,
		maxComponents = 2,
		nodes = 8,
		minEdges = 10,
		maxEdges = 12;*/

	int edges;

	for(int correct = 1;; ++correct) {
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

		if (!(AmB.empty() && BmA.empty())) {
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
		} else if (correct % 10 == 0) {
			cout << "Correct: " << correct << endl;
		}		
	}

	return 0;
}
