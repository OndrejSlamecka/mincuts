#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/graph_generators.h>

using namespace std;
using namespace ogdf;

/**
 * @brief Takes a csv file with lines "<id>;<source>;<target>;<edge name>;..." and transforms it into graph
 * @param sEdgesFileName
 * @return Graph
 */
Graph* csvToGraph(string sEdgesFileName) {
    Graph *G = new Graph;

    ifstream fEdges(sEdgesFileName);
    if (!fEdges.is_open())
        throw new invalid_argument("Edges file doesn't exist or could not be accessed");

    vector<node> nodes;
    int id, u, v, nSize = 0;

    for (string line; getline(fEdges, line);) {
        sscanf(line.c_str(), "%d;%d;%d;", &id, &u, &v);

        if (u > nSize) {
            nodes.resize(u + 1);
            nSize = u;
        }

        if (v > nSize) {
            nodes.resize(v + 1);
            nSize = v;
        }

        if(nodes.at(u) == nullptr)
            nodes[u] = G->newNode(u);

        if(nodes[v] == nullptr)
            nodes[v] = G->newNode(v);

        G->newEdge(nodes[u], nodes[v], id);
    }

    return G;
}


int main()
{
    try {
        Graph *g = csvToGraph("data/silnice.csv");


        delete g;
    } catch (invalid_argument *e) {
        cerr << "Error: " << e->what() << endl;
        return 1;
    }

    return 0;
}

