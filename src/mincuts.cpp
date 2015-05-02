/*
 * Copyright (c) 2015, Ondrej Slamecka <ondrej@slamecka.cz>
 * See the LICENSE file in the root folder of this repository.
 */

#include <iostream>
#include <stdexcept>
#include <string>

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>

#include "./helpers.h"
#include "./circuitcocircuit.h"

using std::string; using std::invalid_argument;
using ogdf::edge; using ogdf::node; using ogdf::Graph; using ogdf::List;
using ogdf::ListConstIterator;

void printUsage(char *name) {
    cerr << "Usage:\t" << name << " <edge_file.csv> " \
         << "<cut size bound> <# of components> [-b]"
#ifdef MEASURE_RUNTIME
         << " [-d]"
#endif
         << "\n\n" \
         << "\t<# of components> can be exact or range (e.g. 2-3)\n" \
         << "\t-b --\tuse bruteforcing of all combinations instead of\n" \
            "\t\tCircuitCocircuit algorithm" << endl;
#ifdef MEASURE_RUNTIME
    cerr << "\t-d --\t<depth of runtime measurement>" << endl;
#endif
}

int main(int argc, char* argv[]) {
    if ((argc == 5 && argv[4] != string("-bfc"))
      || argc < 4
#ifdef MEASURE_RUNTIME
      || argc > 7
#else
      || argc > 5
#endif
      || argv[1] == string("-h") || argv[1] == string("--help")) {
        printUsage(argv[0]);
        exit(1);
    }

    // cutSizeBound, minComponents, maxComponents
    int cutSizeBound, minComponents, maxComponents;
    try {
        cutSizeBound = stoi(argv[2]);

        string thirdArg(argv[3]);

        size_t hyphenPos = thirdArg.find('-');
        if (hyphenPos == string::npos) {
            minComponents = maxComponents = stoi(argv[3]);
        } else {
            minComponents = stoi(thirdArg.substr(0, hyphenPos));
            maxComponents = stoi(thirdArg.substr(hyphenPos + 1));

            if (maxComponents < minComponents) {
                cerr << "max # of components < min # of components" << endl;
                exit(2);
            }
        }
    } catch (invalid_argument &_) {  // stoi failed
        printUsage(argv[0]);
        exit(1);
    }

    // Algorithm to use
    bool useCircuitCocircuit = true;
    if (argc >= 5 && argv[4] == string("-b")) {
        useCircuitCocircuit = false;
    }

#ifdef MEASURE_RUNTIME
    int measurementDepth = 2;
    if (useCircuitCocircuit && argc == 6 && argv[4] == (string) "-d") {
        measurementDepth = stoi(argv[5]);
    } else if (!useCircuitCocircuit && argc == 7 && argv[5] == (string) "-d") {
        measurementDepth = stoi(argv[6]);
    } else {
        measurementDepth = maxComponents;
    }
#endif

    // Input data
    Graph G;
    ifstream fGraph(argv[1]);
    if (!fGraph.is_open()) {
        cerr << "Graph file " << argv[1] << " doesn't exist or could not be " \
                "accessed. Terminating." << endl;
        exit(3);
    }
    csv2graph(G, fGraph);

    try {
        if (useCircuitCocircuit) {
#ifdef MEASURE_RUNTIME
            CircuitCocircuit alg(G, cutSizeBound, measurementDepth);
#else
            CircuitCocircuit alg(G, cutSizeBound);
#endif

            for (int i = minComponents; i <= maxComponents; ++i) {
                alg.run(i);
            }
        } else {
            List<List<edge>> bonds;
            bruteforceGraphBonds(G, cutSizeBound, minComponents, maxComponents,
                                 bonds);

            forall_listiterators(List<edge>, it, bonds) {
                cout << *it << endl;
            }
        }
    } catch (invalid_argument &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}

