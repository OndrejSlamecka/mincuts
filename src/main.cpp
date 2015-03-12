/**
 *
 * Terminology (Matroid - Graph) for escalated version of algorithm:
 *  Basis - Spanning forest
 *  Independent set - Tree (i.e. subset of a basis)
 *  Circuit - cycle
 *  Cocircuit - minimal edge-cut
 *  Hyperplane - maximal set not containing any basis (= complement of a min-cut)
 *
 *  Spanning forest - union of spanning trees of each component of an unconnected graph
 *
 */

#include <iostream>
#include <stdexcept>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>

#include "helpers.h"
#include "circuitcocircuit.h"

using namespace std;
using namespace ogdf;

// bruteforce alg:

// note that these are actually variations
// TODO: some sort of generating combinations such that we get all of size s,
// s.t. 1 <= s <= k...
// IDEA: Generate all comb. of size i and in next iteration use those to build comb. of size i+1? Do later...
void combinations(Graph &G, const ogdf::List<edge> &allEdges,
        const ogdf::List<edge>::iterator &start, const ogdf::List<edge> &acc, int k,
        int components)
{
    int ncomponents;
    if (isMinCut(G, acc, ncomponents) == 0 && ncomponents <= components) {
		cout << acc << endl;
    }
	if (k == 0) {
		return;
	}

    // TODO: Improve
    ogdf::List<edge> lessEdges(allEdges), moreAcc(acc);
    for (edge e : allEdges) {
//    for (List<edge>::iterator it = lessEdges.begin(); it != lessEdges.end(); ++it) {
//        edge e = *it;
        lessEdges.removeFirst(e);
		moreAcc.pushBack(e);

        combinations(G, lessEdges, start, moreAcc, k - 1, components);

		moreAcc.removeFirst(e);
        lessEdges.pushBack(e);
	}
}

void bruteforce(Graph &G, int cutSizeBound, int components)
{
    ogdf::List<edge> allEdges;
	G.allEdges(allEdges);

    ogdf::List<edge> acc;
    combinations(G, allEdges, allEdges.begin(), acc, cutSizeBound, components);
}

void printUsage(char *name) {
    cout << "Usage:\t" << name << " <edge_file.csv> " \
            "<cut size bound> <max components> [-bfc]" << endl;
	cout << endl \
   		 << "\t-bfc --\tuse bruteforcing of all combinations instead of\n" \
            "\t\tcircuit-cocircuit algorithm" << endl;
}

int main(int argc, char* argv[])
{
    if ((argc == 5 && argv[4] != string("-bfc"))
      || argc < 4 || argc > 5
	  || argv[1] == string("-h") || argv[1] == string("--help")) {
        printUsage(argv[0]);
        exit(1);
    }

    int algorithm = 0; // 0 for CircuitCocircuit, 1 for bruteforce
    if (argc == 5 && argv[4] == string("-bfc")) {
        algorithm = 1;
    }

    ifstream fEdges(argv[1]);
    if (!fEdges.is_open()) {
        cerr << "Edges file doesn't exist or could not be accessed. Terminating." << endl;
        exit(2);
    }

    int cutSizeBound, components;
    try {
        cutSizeBound = stoi(argv[2]);
        components = stoi(argv[3]);
    } catch (invalid_argument &_) { // stoi failed
        printUsage(argv[0]);
        exit(1);
    };

    try {
        Graph G;
        csvToGraph(G, fEdges);

        if (algorithm == 1) {
            bruteforce(G, cutSizeBound, components);
        } else {
            ogdf::List<ogdf::List<edge>> bonds;

            CircuitCocircuit alg(G, cutSizeBound);

            for (int i = 2; i <= components; ++i) {
                alg.run(i, bonds); // i, bonds
            }

            //alg.run(2, bonds);

            for(ogdf::List<ogdf::List<edge> >::iterator it = bonds.begin(); it != bonds.end(); ++it) {
				cout << *it << endl;
            }
        }

    } catch (invalid_argument &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}

