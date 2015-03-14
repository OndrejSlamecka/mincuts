#include <iostream>
#include <stdexcept>
#include <vector>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>

#include "helpers.h"
#include "circuitcocircuit.h"

using namespace std;
using namespace ogdf;

void printUsage(char *name) {
    cout << "Usage:\t" << name << " <edge_file.csv> " \
            "<cut size bound> <# of components> [-bfc]" << endl;
	cout << endl \
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

    ifstream fEdges(argv[1]);
    if (!fEdges.is_open()) {
        cerr << "Edges file doesn't exist or could not be accessed. " \
                "Terminating." << endl;
        exit(2);
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

            if (minComponents > maxComponents) {
                cerr << "Given range for number of components has negative " \
                        "length. a >= b has to hold in given range 'a-b'." \
                     << endl;
                exit(3);
            }
        }
    } catch (invalid_argument &_) { // stoi failed
        printUsage(argv[0]);
        exit(1);
    };

    try {
        Graph G;
        csvToGraph(G, fEdges);

        if (algorithm == 1) {
            List<List<edge>> bonds;
            bruteforceGraphBonds(G, cutSizeBound, minComponents, maxComponents, bonds);

            for(List<List<edge>>::iterator it = bonds.begin(); it != bonds.end(); ++it) {
                cout << *it << endl;
            }
        } else {
            List<bond> bonds;
            CircuitCocircuit alg(G, cutSizeBound);

            for (int i = minComponents; i <= maxComponents; ++i) {
                alg.run(i, bonds);
            }

            for(List<bond>::iterator it = bonds.begin(); it != bonds.end(); ++it) {
                cout << *it << endl;
            }
        }

    } catch (invalid_argument &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}

