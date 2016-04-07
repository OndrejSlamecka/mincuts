/*
 * Copyright (c) 2015, Ondrej Slamecka <ondrej@slamecka.cz>
 * See the LICENSE file in the root folder of this repository.
 */
/*
 * Note that this tool consumes quite a lot of memory when run on big inputs,
 * however it has not been a problem for what we needed yet
 */


#define BOOST_CHRONO_VERSION 2
#include <boost/chrono.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/graph_generators.h>

#include <iostream>
#include <stdexcept>
#include <memory>
#include <set>

#include "circuitcocircuit.h"

using namespace std;
using namespace ogdf;

typedef boost::chrono::process_user_cpu_clock clock_type;
typedef boost::chrono::time_point<clock_type> time_point;

// The GIST:
// for i in N..c*N, c is a constant
//   for j in 1..<# of experiments>
//     G = random graph
//     make conn. G
//     create object
//     measure start
//     run circuit cocircuit
//     measure end

void printUsage(char *name) {
    cerr << "Usage: " << name << "<|V|> <max |E|> <t> <m> <k>\n"
         << "\n\t t -- number of experiments per edge count"
         << "\n\t m -- cut size bound"
         << "\n\t k -- multiplicity of bonds" << endl;
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printUsage(argv[0]);
        exit(1);
    }

    int nVertices, nMaxEdges, nExperiments, cutSizeBound, nCutMultiplicity;
    try {
        nVertices = stoi(argv[1]);
        nMaxEdges = stoi(argv[2]);
        nExperiments = stoi(argv[3]);
        cutSizeBound = stoi(argv[4]);
        nCutMultiplicity = stoi(argv[5]);
    } catch (invalid_argument &_) {  // stoi failed
        printUsage(argv[0]);
        exit(1);
    }

    for (int m = nVertices; m <= nMaxEdges; m += 1) {
        for (int j = 0; j < nExperiments; ++j) {
            Graph G;
            randomSimpleGraph(G, nVertices, m);
            List<edge> added;
            makeConnected(G, added);

            CircuitCocircuit alg(G, cutSizeBound);
            List<bond> bonds;

            time_point start = boost::chrono::process_user_cpu_clock::now();
            alg.run(nCutMultiplicity, bonds);
            time_point end = boost::chrono::process_user_cpu_clock::now();

            cout << G.numberOfEdges() << "\t"
                 << boost::chrono::duration_cast<boost::chrono::milliseconds>(end - start).count()
                    << "\t" << bonds.size() << "\n";
        }
    }

    return 0;
}
