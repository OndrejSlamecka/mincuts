// TODO: Run generators in parallel with tester

#include <iostream>
#include <stdexcept>
#include <memory>
#include <set>
#include <chrono>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/graph_generators.h>
#include "helpers.h"
#include "circuitcocircuit.h"

#ifdef NAUTY
#include <gtools.h>
int GENG_MAIN(int argc, char *argv[]);
#endif

using namespace ogdf;
using std::string;
using std::stoi;
using std::ostream;
using std::cout;
using std::cerr;
using std::unique_ptr;
using std::invalid_argument;
using std::pair; using std::make_pair;

void printUsage(char *name)
{
	cerr << "This program tests CircuitCocircuit implementation.\n" \
		 << "Usage:\t" << name << "\t<cut size bound> <# of components> " \
		 << "[-c, --canonical]\n" \
		 << "\t\t[-r, --randomized <# of nodes> <min>-<max edges>]\n\n";

	cerr << "\tFirst two arguments are used for any graph being tested.\n"
		 << "\t<# of components> can be exact or range (e.g. 2-3)\n" \
		 << "\t-r generate random graphs\n" \
		 << "\t<max edges> is not strict (if the random graph is " \
		 << "disconnected\n\t\tthen we add edges to connect it)\n" \
		 << "\t-c generates graphs cannonically using nauty\n";

#ifndef NAUTY
	cerr << "\t\t(you need to recompile tester with -DNAUTY flag)\n";
#endif
}

ostream & operator<<(ostream &os, const std::set<int>& S)
{
	int i = 0, ss = S.size();
	for (auto e : S) {
		os << e;
		if (i < ss - 1) os << ",";
		i++;
	}
	return os;
}

class GraphGenerator {
public:
	GraphGenerator() {};
	virtual void get(Graph &G) = 0;
	virtual ~GraphGenerator() {};
};

class RandomGraphGenerator : public GraphGenerator
{
	int nodes;
	int minEdges, maxEdges;

public:
	RandomGraphGenerator(int nodes, int min, int max)
		: nodes(nodes), minEdges(min), maxEdges(max) {};

	/**
	 * OGDF's solution is probably biased... TODO
	 * http://stackoverflow.com/a/14618505/247532
	 */
	void get(Graph &G) {
		int edges = randomNumber(minEdges, maxEdges);
		randomSimpleGraph(G, nodes, edges);
		List<edge> added;
		makeConnected(G, added);
	}

	~RandomGraphGenerator() {};
};

#ifdef NAUTY
// Due to design of geng we need to use global variables
// (How nice would it be if we could pass a callback to geng routine?)
SList<pair<int,graph*>> nautyGeneratedGraphs;

void OUTPROC(FILE *outfile, graph *g, int n)
{
	nautyGeneratedGraphs.pushBack(make_pair(n,g));
}

class CanonicalGraphGenerator : public GraphGenerator
{
private:
	int nodes = 3; // start with 3 nodes, increase with each level

	void generate() {
		int geng_argc;
		char *geng_argv[4];
		// number of digits of biggest integer, in reality
		// "nodes" will never exceed 100 so 3 should be enough
		char sNodes[64];
		sprintf(sNodes, "%d", nodes);

		geng_argv[0] = (char *) "geng";
		geng_argv[1] = (char *) "-qc"; // quiet geng, connected graphs
		geng_argv[2] = sNodes;
		geng_argv[3] = NULL;
		geng_argc = 3;
		GENG_MAIN(geng_argc, geng_argv);

		nodes++;
	}

	/**
	 * Converts from nauty graph format to ogdf's Graph
	 */
	void graphToGraph(int n, graph *g, Graph &G) {
		G.clear();
		Array<node> vertices(n);

		for (int i = 0; i < n; ++i) {
			vertices[i] = G.newNode(i);
		}

		set *gj; // nauty's set, not from standard library
		for (int j = 1; j < n; ++j) {
			gj = GRAPHROW(g, j, 1);

			for (int i = 0; i < j; ++i) {
				if (ISELEMENT(gj,i)) {
					G.newEdge(vertices[j], vertices[i]);
				}
			}
		}
	}

public:
	CanonicalGraphGenerator() {};
	void get(Graph &G) {
		if (nautyGeneratedGraphs.empty()) {
			cout << "Asking geng for graphs on " << nodes << " nodes" << endl;
			generate();
		}

		pair<int, graph*> ng = nautyGeneratedGraphs.popFrontRet();
		graphToGraph(ng.first, ng.second, G);
	}

	~CanonicalGraphGenerator() {};
};
#endif

unique_ptr<RandomGraphGenerator> prepareRandomizedTesting(char *argv[])
{
	int nodes, minEdges, maxEdges;
	try {
		size_t hyphenPos;

		// # of nodes
		nodes = stoi(argv[4]);

		// min-max edges
		string fifthArg(argv[5]);
		hyphenPos = fifthArg.find('-');
		if (hyphenPos == string::npos) {
			throw invalid_argument("Number of edges is not range.");
		}
		minEdges = stoi(fifthArg.substr(0, hyphenPos));
		maxEdges = stoi(fifthArg.substr(hyphenPos + 1));

		if (maxEdges < minEdges) {
			throw invalid_argument("max edges < min edges");
		}

	} catch(invalid_argument &_) { // stoi failed
		printUsage(argv[0]);
		exit(2);
	}

	auto rgg = new RandomGraphGenerator(nodes, minEdges, maxEdges);
	return unique_ptr<RandomGraphGenerator>(rgg);
}

int main(int argc, char *argv[])
{
	std::ios_base::sync_with_stdio(false);

	if (argc != 6 && argc != 4) {
		printUsage(argv[0]);
		exit(1);
	}

	// Configuring bond parameters
	int cutSizeBound, minComponents, maxComponents;
	try {
		// cut size bound
		cutSizeBound = stoi(argv[1]);

		// # of components
		string secondArg(argv[2]);
		size_t hyphenPos = secondArg.find('-');
		if (hyphenPos == string::npos) {
			minComponents = maxComponents = stoi(argv[2]);
		} else {
			minComponents = stoi(secondArg.substr(0, hyphenPos));
			maxComponents = stoi(secondArg.substr(hyphenPos + 1));
		}

		if (maxComponents < minComponents) {
			throw invalid_argument("max components < min components");
		}
	} catch(invalid_argument &_) { // stoi failed
		printUsage(argv[0]);
		exit(2);
	}

	unique_ptr<GraphGenerator> gg;
	if (argv[3] == string("-r") || argv[3] == string("--randomized")) {
		gg = prepareRandomizedTesting(argv);
	} else if (argv[3] == string("-c") || argv[3] == string("--canonical")) {
#ifdef NAUTY
		gg = unique_ptr<CanonicalGraphGenerator>(new CanonicalGraphGenerator());
#else
		cerr << "This program was not compiled with nauty, " \
			 << "compile again with -DNAUTY flag." << endl;
		exit(3);
#endif
	} else {
		cerr << "Unrecognized option " << argv[3] << "." << endl;
		printUsage(argv[0]);
		exit(4);
	}

	std::chrono::time_point<std::chrono::system_clock> lastOutput, now;
	lastOutput = std::chrono::system_clock::now();

	for(int correct = 0;; ++correct) {
		Graph G;
		gg->get(G);

		// run circuitcocircuit and bfc
		List<bond> bonds;
		CircuitCocircuit alg(G, cutSizeBound);
		for (int i = minComponents; i <= maxComponents; ++i) {
			alg.run(i, bonds);
		}

		List<List<edge>> bf_bonds;
		bruteforceGraphBonds(G, cutSizeBound, minComponents, maxComponents,
				bf_bonds);

		std::set<std::set<int>> A, B, AmB, BmA;
		for(bond b : bonds) {
			std::set<int> si;
			for (edge e : b.edges) si.insert(e->index());
			A.insert(si);
		}

		for(List<edge> le : bf_bonds) {
			std::set<int> si;
			for (edge e : le) si.insert(e->index());
			B.insert(si);
		}

		std::set_difference(A.begin(), A.end(), B.begin(), B.end(),
			std::inserter(AmB, AmB.end()));
		std::set_difference(B.begin(), B.end(), A.begin(), A.end(),
			std::inserter(BmA, BmA.end()));

		now = std::chrono::system_clock::now();
		if (AmB.empty() && BmA.empty()) {
			if (std::chrono::duration_cast<std::chrono::seconds>(now-lastOutput).count() > 1) {
				cout << "Correct: " << correct << endl;
				lastOutput = now;
			}
		} else {
			cout << "\nmincuts / bf:" << endl;
			for (std::set<int> c : AmB) {
				std::cout << c << endl;
			}

			cout << "bf / mincuts:" << endl;
			for (std::set<int> c : BmA) {
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
