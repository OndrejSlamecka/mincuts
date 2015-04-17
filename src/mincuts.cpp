#include <iostream>
#include <stdexcept>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>

#include "helpers.h"
#include "circuitcocircuit.h"

using namespace std;
using namespace ogdf;

void printUsage(char *name)
{
    cerr << "Usage:\t" << name << " <edge_file.csv> " \
         << "<cut size bound> <# of components> [-bfc] \n" \
         << "\t<# of components> can be exact or range (e.g. 2-3)\n" \
         << "\t-bfc --\tuse bruteforcing of all combinations instead of\n" \
            "\t\tCircuitCocircuit algorithm" << endl;
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
    } catch (invalid_argument &_) { // stoi failed
        printUsage(argv[0]);
        exit(1);
    };

    Graph G;
    ifstream fGraph(argv[1]);
    if (!fGraph.is_open()) {
        cerr << "Graph file " << argv[1] << " doesn't exist or could not be " \
                "accessed. Terminating." << endl;
        exit(3);
    }
    csv2graph(G, fGraph);

    try {
        if (algorithm == 1) {
            List<List<edge>> bonds;
            bruteforceGraphBonds(G, cutSizeBound, minComponents, maxComponents, bonds);

            for(List<List<edge>>::iterator it = bonds.begin(); it != bonds.end(); ++it) {
                cout << *it << endl;
            }
        } else {            
            CircuitCocircuit alg(G, cutSizeBound);

            for (int i = minComponents; i <= maxComponents; ++i) {
                alg.run(i);
            }            
        }
    } catch (invalid_argument &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}

