/*
 * Copyright (c) 2015, Ondrej Slamecka <ondrej@slamecka.cz>
 * See the LICENSE file in the root folder of this repository.
 */

#include "./helpers.h"
#include <string>
#include <algorithm>  // min
#include <vector>
#include <set>

using std::ifstream; using std::ostream; using std::stringstream;
using std::vector; using std::set; using std::string;
using std::cout; using std::cerr; using std::endl;
using std::min;
using ogdf::edge; using ogdf::node; using ogdf::Graph; using ogdf::List;
using ogdf::NodeArray; using ogdf::ListConstIterator;

/**
 * Reads a csv file with lines "<id>;<source>;<target>;..." and transforms it into a graph
 */
void csv2graph(Graph &G, ifstream &fEdges) {
    string line;

    int id, u, v;
    int maxNodeId = 0;
    for (; getline(fEdges, line);) {
        sscanf(line.c_str(), "%d;%d;%d;", &id, &u, &v);
        if (u > maxNodeId) maxNodeId = u;
        if (v > maxNodeId) maxNodeId = v;
    }

    // clear should not be needed in C++11 but as it seems it actually is
    fEdges.clear();
    fEdges.seekg(0);

    vector<node> nodes(maxNodeId + 1);

    for (; getline(fEdges, line);) {
        sscanf(line.c_str(), "%d;%d;%d;", &id, &u, &v);

        if (nodes[u] == nullptr) {
            nodes[u] = G.newNode(u);
        }

        if (nodes[v] == nullptr) {
            nodes[v] = G.newNode(v);
        }

        G.newEdge(nodes[u], nodes[v], id);
    }
}

void graph2csv(const Graph &G, ostream &fGraph) {
    for (edge e : G.edges) {
        fGraph << e->index() << ";" << e->source() << ";" << e->target()
               << endl;
    }
}

void graph2dot(const Graph &G, ostream &fGraph) {
    fGraph << "graph G {" << endl;

    for (edge e : G.edges) {
        fGraph << "\t" << e->source() << " -- " << e->target()
               << " [label \"" << e->index() << "\"]" << endl;
    }

    fGraph << "}" << endl;
}

ostream & operator<<(ostream &os, const set<edge> &S) {
    // TODO: Reduce to  operator<<(ostream &os, const std::set<int>& S)
    int i = 0, ss = S.size();
    for (auto e : S) {
        os << e->index();
        if (i < ss - 1) os << ",";
        i++;
    }
    return os;
}

ostream & operator<<(ostream &os, const std::set<int>& S) {
    int i = 0, ss = S.size();
    for (auto e : S) {
        os << e;
        if (i < ss - 1) os << ",";
        i++;
    }
    return os;
}

ostream & operator<<(ostream &os, const List<edge> &L) {
    int i = 0, ls = L.size();
    for (auto e : L) {
        os << e->index();
        if (i < ls - 1) os << ",";
        i++;
    }
    return os;
}

string nameColor(Colour c) {
    switch (c) {
        case Colour::BLACK: return "black";
        case Colour::RED: return "red";
        case Colour::BLUE: return "blue";
        default: return "ERROR";
    }
}

string nodesByColor2str(const Graph &G, const GraphColouring &coloring,
                        Colour c) {
    stringstream s;

    int x = 0;
    for (node n : G.nodes) {
        if (coloring[n] == c) {
            x++;
        }
    }

    for (node n : G.nodes) {
        if (coloring[n] == c) {
            s << n->index();
            x--;
            if (x != 0) {
                s << ", ";
            }
        }
    }

    return s.str();
}


string edgesByColor2str(const Graph &G, const GraphColouring &coloring,
                        Colour c) {
    stringstream s;

    int x = 0;
    for (edge e : G.edges) {
        if (coloring[e] == c) {
            x++;
        }
    }

    for (edge e : G.edges) {
        if (coloring[e] == c) {
            s << e->index();
            x--;
            if (x > 0) {
                s << ", ";
            }
        }
    }

    return s.str();
}

string coloring2str(const Graph &G, const GraphColouring &c) {
    stringstream s;

    s << "Nodes: "
      << "(blue) " << nodesByColor2str(G, c, Colour::BLUE) << "; "
      << "(red) " << nodesByColor2str(G, c, Colour::RED) << "; "
      << "(black) " << nodesByColor2str(G, c, Colour::BLACK) << "; ";

    s << "Edges: "
      << "(blue) " << edgesByColor2str(G, c, Colour::BLUE) << "; "
      << "(red) " << edgesByColor2str(G, c, Colour::RED) << "; "
      << "(black) " << edgesByColor2str(G, c, Colour::BLACK) << "; ";

    return s.str();
}

string edgelist2str(const ogdf::List<edge> &edges) {
    stringstream s;
    for (auto it = edges.begin(); it != edges.end(); ++it) {
        s << (*it)->index();
        if (*it != edges.back()) {
            s << ", ";
        }
    }
    return s.str();
}

edge edgeByIndex(const List<edge> &edges, int index) {
    for (edge e : edges) {
        if (e->index() == index) {
            return e;
        }
    }
    return nullptr;
}

void indicies2edges(const List<edge> &graphEdges, const string &str,
                    List<edge> &l) {
    stringstream ss(str);
    string item;
    while (getline(ss, item, ',')) {
        int index = stoi(item);
        edge e = edgeByIndex(graphEdges, index);
        l.pushBack(e);
    }
}

/**
 * Helper function to determine whether given set of edges really is a cut
 */
bool isCut(Graph &G, const List<edge> &cut) {
    Graph::HiddenEdgeSet hidden_cut(G);

    for (auto e : cut) {
        hidden_cut.hide(e);
    }

    bool r = isConnected(G);

    hidden_cut.restore();
    return !r;
}

/**
 * Helper function to determine whether given set of edges really is a minimal
 *  cut, w.r.t. # of components of (G \ cut)
 * Returns
 *     0 on success,
 *     -1 if # of components is 1,
 *     the number of components of (G \ cut) otherwise
 */
int isMinCut(Graph &G, const List<edge> &cut, int &ncomponents) {
    NodeArray<int> component(G);
    Graph::HiddenEdgeSet hidden_edges(G);

    for (auto e : cut) {
        hidden_edges.hide(e);
    }
    ncomponents = connectedComponents(G, component);

    if (ncomponents == 1) {
        hidden_edges.restore();
        return -1;
    }

    for (auto e : cut) {
        hidden_edges.restore(e);

        // Count # of components of smaller cut
        int nSmallerCutComponents = connectedComponents(G, component);

        // If numbers of components is the same and it's still a cut then
        // obviously the original cut was not minimal
        if (nSmallerCutComponents == ncomponents) {
            hidden_edges.restore();
            return ncomponents;
        }
        hidden_edges.hide(e);
    }

    hidden_edges.restore();

    return 0;
}

int isMinCut(Graph &G, const List<edge> &cut) {
    int nc;
    return isMinCut(G, cut, nc);
}

void bruteforceGraphBonds(Graph &G, int cutSizeBound, int minComponents,
                int maxComponents, List<List<edge>> &bonds) {
    List<edge> allEdges;
    G.allEdges(allEdges);

    int n = allEdges.size();

    int stop = min(cutSizeBound, G.numberOfEdges());
    for (int k = 1; k <= stop; ++k) {
        vector<bool> v(n);
        fill(v.begin() + n - k, v.end(), true);

        // http://stackoverflow.com/a/9430993
        do {
            List<edge> c;
            for (int i = 0; i < n; ++i) {
                if (v[i]) {
                    c.pushBack(*allEdges.get(i));
                }
            }

            int ncomponents;
            if (isMinCut(G, c, ncomponents) == 0
             && minComponents <= ncomponents && ncomponents <= maxComponents) {
                bonds.pushBack(c);
            }
        } while (next_permutation(v.begin(), v.end()));
    }
}
