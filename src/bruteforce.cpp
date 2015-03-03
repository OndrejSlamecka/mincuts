#include <iostream>
#include <stdexcept>
#include <ogdf/basic/Queue.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Stack.h>
#include <ogdf/basic/simple_graph_alg.h>

#include "helpers.hpp"
#include "graphcoloring.h"

using namespace std;
using namespace ogdf;

// Global variables are evil but these two never change...
int cutSizeBound;
int components;

List<List<edge>> bruteforce(Graph &G, List<edge> edges, int k) {
    List<List<edge>> r;
    List<edge> empty;
    r.pushBack(empty);

    if (k == 0 || edges.empty()) return r;

    edge firstEdge = edges.front();
    edges.removeFirst(edges.front());

    auto combs = bruteforce(G, edges, k - 1);
    for (List<edge> c : combs) {
        if (c.size() >= k-1) {
            c.pushFront(firstEdge);
            r.pushBack(c);
        }
    }

    combs = bruteforce(G, edges, k);
    for (auto c : combs) {
        if (!c.empty()) {
            r.pushBack(c);
        }
    }

    return r;
}

int main(int argc, char **argv) {
    if (argc != 4 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        cout << "Usage: " << argv[0] << " <graph.csv> <cocircuit size bound> <components>" << endl;
        exit(2);
    }

    ifstream fEdges(argv[1]);
    if (!fEdges.is_open()) {
        cerr << "Edges file doesn't exist or could not be accessed. Terminating." << endl;
        exit(3);
    }

    cutSizeBound = stoi(argv[2]);
    if (cutSizeBound < 1) {
        cerr << "Cocircuit size bound lower than 1. Terminating." << endl;
        exit(4);
    }

    components = stoi(argv[3]);
    if (components < 1) {
        cerr << "Desired components number lower than 1. Terminating." << endl;
        exit(5);
    }


    try {
        Graph G;
        csvToGraph(G, fEdges);

        // idea: take all combinations of (edges over i) for all i <= # components
        // try them -> if they disconnect the graph

        List<List<edge>> bonds;
        List<edge> c;

        List<edge> edges;
        G.allEdges(edges);
        bonds = bruteforce(G, edges, cutSizeBound);

        NodeArray<int> component(G);
        for(List<List<edge> >::iterator it = bonds.begin(); it != bonds.end(); ++it) {            
            for (auto e : *it) {
                G.hideEdge(e);
            }

            int ncomps = connectedComponents(G, component);
            if (1 < ncomps && ncomps <= components) {
                cout << *it << " " << ncomps << endl;
            }

            G.restoreAllEdges();
        }

    } catch (invalid_argument *e) {
        cerr << "Error: " << e->what() << endl;
        return 1;
    }

    return 0;
}
