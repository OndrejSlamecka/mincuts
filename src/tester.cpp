/*
 * Copyright (c) 2015, Ondrej Slamecka <ondrej@slamecka.cz>
 * See the LICENSE file in the root folder of this repository.
 */

#include <iostream>
#include <stdexcept>
#include <memory>
#include <set>
#include <chrono>
#include <mutex>
#include <thread>
#include <atomic>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/graph_generators.h>
#include "helpers.h"
#include "circuitcocircuit.h"

#define BIAS6 63
#define SMALLN 62
#define TOPBIT6 32

using namespace ogdf;
using namespace std;

/* Graph sources */

/**
 * Since the default updateOnGraphRetrieval() uses no synchronization
 * the user might get updates on the number of retrieved graphs more often
 * than once a second if the extending classes do not implement any lock
 * (e.g. RandomGraphSource does this)
 */
class AbstractGraphSource
{
	std::chrono::time_point<std::chrono::system_clock> lastOutputTime;
	atomic<int> nGraphsRetrieved = ATOMIC_VAR_INIT(0);

protected:
	AbstractGraphSource()
	{
		lastOutputTime = std::chrono::system_clock::now();
	}

	void updateOnGraphRetrieval()
	{
		++nGraphsRetrieved;
		auto now = std::chrono::system_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::seconds>
				(now-lastOutputTime).count();
		if (diff > 1) {
			cout << "Graphs retrieved: " << nGraphsRetrieved.load() << endl;
			lastOutputTime = now;
		}
	}

public:

	virtual bool get(Graph &G) = 0;
	virtual ~AbstractGraphSource() {}
};


/**
 * Reads graphs in nauty's graph6 format from stdin.
 */
class StdinGraphSource : public AbstractGraphSource
{
	mutex cin_mtx;

	/*
	 * The following two functions are from the nauty package.
	 * See http://pallini.di.uniroma1.it/
	 * stringtograph was modified to work with OGDF's Graph class
	 */

	/**
	 * Get size of graph out of graph6 or sparse6 string.
	 */
	static int graphsize(char *s)
	{
		char *p;
		int n;

		if (s[0] == ':') p = s+1;
		else             p = s;
		n = *p++ - BIAS6;

		if (n > SMALLN)	{
			n = *p++ - BIAS6;
			n = (n << 6) | (*p++ - BIAS6);
			n = (n << 6) | (*p++ - BIAS6);
		}
		return n;
	}

	/**
	 * Convert string in graph6 format to graph.
	 */
	static void	stringtograph(char *s, Graph &G)
	{
		int n = graphsize(s), x = 0;

		char *p = s + 1;
		if (n > SMALLN) p += 3;

		vector<node> vertices(n);
		for(int i = 0; i < n; ++i) {
			vertices[i] = G.newNode(i);
		}

		int k = 1;
		for (int j = 1; j < n; ++j) {
			for (int i = 0; i < j; ++i) {
				if (--k == 0) {
					k = 6;
					x = *(p++) - BIAS6;
				}

				if (x & TOPBIT6) {
					G.newEdge(vertices[i], vertices[j]);
				}
				x <<= 1;
			}
		}
	}

public:

	/**
	 * Locks cin!
	 */
	bool get(Graph &G)
	{
		string s;
		unique_lock<mutex> lock(cin_mtx);
		bool r = getline(cin, s);
		updateOnGraphRetrieval();
		lock.unlock();

		if (r) {
			char *cp = (char*) s.c_str();
			stringtograph(cp, G);
		}

		return r;
	}
};

class RandomGraphSource : public AbstractGraphSource
{
	int nodes, minEdges, maxEdges;


public:
	RandomGraphSource(int nodes, int minE, int maxE)
		: nodes(nodes), minEdges(minE), maxEdges(maxE)
	{
		srand (time(NULL)); // OGDF uses rand()
	}

	/**
	 * OGDF's solution is probably biased,
	 * (see http://stackoverflow.com/a/14618505/247532) but we don't care
	 * since other solutions are way slower
	 */
	bool get(Graph &G)
	{
		int edges = randomNumber(minEdges, maxEdges);

		randomSimpleGraph(G, nodes, edges);
		List<edge> added;
		makeConnected(G, added);

		updateOnGraphRetrieval();
		return true;
	}
};


/* Test runner */

class TestRunner
{
	unique_ptr<AbstractGraphSource> graphs;

protected:
	mutex errorOutputMutex;
	bool errorFound = false;

	int minComponents, maxComponents,
		cutSizeBound;

	void reportProblem(const Graph &G, const set<set<int>> &AmB, const set<set<int>> &BmA)
	{
		unique_lock<mutex> lock(errorOutputMutex, defer_lock);
		if (errorFound || !lock.try_lock()) {
			// Somebody is already reporting a problem
			return;
		}
		errorFound = true;

		cout << "\nmincuts / bf:" << endl;
		for (const set<int> &c : AmB) {
			cout << c << endl;
		}

		cout << "bf / mincuts:" << endl;
		for (const set<int> &c : BmA) {
			cout << c << endl;
		}

		ofstream fGraph("tmp/tester_in.csv");
		if (!fGraph.is_open()) {
			cerr << "tmp/tester_in.csv could not be opened" << endl;
			exit(3);
		}
		graph2csv(G, fGraph);
		cout << "Input graph written to tmp/tester_in.csv" << endl;
	}

	bool testGraph(Graph &G)
	{
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
		for(const bond &b : bonds) {
			set<int> si;
			for (edge e : b.edges) si.insert(e->index());
			A.insert(si);
		}

		for(const List<edge> &le : bf_bonds) {
			set<int> si;
			for (edge e : le) si.insert(e->index());
			B.insert(si);
		}

		set_difference(A.begin(), A.end(), B.begin(), B.end(),
				inserter(AmB, AmB.end()));
		set_difference(B.begin(), B.end(), A.begin(), A.end(),
				inserter(BmA, BmA.end()));

		if (AmB.empty() && BmA.empty()) {
			return true;
		} else {
			reportProblem(G, AmB, BmA);
			return false;
		}
	}

public:
	TestRunner(int minC, int maxC, int cutSizeB, unique_ptr<AbstractGraphSource> graphs)
		: graphs(move(graphs)), minComponents(minC), maxComponents(maxC), cutSizeBound(cutSizeB)
	{ }

	virtual void run() final
	{
		while(!errorFound) {
			Graph G;

			// Exhausted input?
			if (!graphs->get(G)) {
				break;
			}

			// Problem found on G?
			if (!testGraph(G)) {
				break;
			}
		}
	}
};

/* --- */

void printUsage(char *name)
{
	cerr << "Usage:\t" << name << " <cut size bound> <# of components> " \
		<< "\n\t\t[-r, --randomized <# of nodes> <min>-<max edges>] [-tN]\n\n";

	cerr << "\tThis program tests CircuitCocircuit implementation.\n" \
		<< "\tExpects graphs in nauty's graph6 format on stdin.\n\n" \

		<< "\tFirst two arguments are used for any graph being tested.\n" \
		<< "\t<# of components> can be exact or range (e.g. 2-3)\n" \
		<< "\t-r generate random graphs (won't use stdin input)\n" \
		<< "\t<max edges> is not strict (if the random graph is " \
		<< "disconnected\n\t\tthen we add edges to connect it)\n" \
		<< "\t-tN use N threads (by default N == 2)\n";
}

int main(int argc, char *argv[])
{
	if (argc < 3 || argc == 5 || argc > 7) {
		printUsage(argv[0]);
		exit(1);
	}

	// Number of components and cut size bound settings
	int minComponents, maxComponents, cutSizeBound;
	try {
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

	// Graph source settings
	unique_ptr<AbstractGraphSource> source;

	if (argc == 3 || (argv[3] != string("-r") && argv[3] != string("--randomized"))) {
		source = unique_ptr<StdinGraphSource>(new StdinGraphSource());
	} else {
		// Configure randomized testing
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

		source = unique_ptr<RandomGraphSource>(new RandomGraphSource(nodes, minEdges, maxEdges));
	}

	// Parallelization settings
	int nThreads = 2;
	string lastArg(argv[argc - 1]);
	if (lastArg.substr(0, 2) == "-t") {
		nThreads = stoi(lastArg.substr(2));
	}

	// Run tester in parallel
	try {
		TestRunner testrunner(minComponents, maxComponents, cutSizeBound, move(source));

		vector<thread> threads(nThreads);
		for (int i = 0; i < nThreads; ++i) {
			threads[i] = thread(&TestRunner::run, ref(testrunner));
		}

		for (int i = 0; i < nThreads; ++i) {
			threads[i].join();
		}
	} catch (const exception& e) {
		cerr << e.what() << endl;
		exit(3);
	}

	return 0;
}
