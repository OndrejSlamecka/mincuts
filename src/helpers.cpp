#include "helpers.h"

using namespace std;
using namespace ogdf;

/**
 * Reads a csv file with lines "<id>;<source>;<target>;..." and transforms it into a graph
 */
void csv2graph(Graph &G, ifstream &fEdges)
{
    string line;

    int id, u, v;
    int maxNodeId = 0;
    for (; getline(fEdges, line);) {
        sscanf(line.c_str(), "%d;%d;%d;", &id, &u, &v);
        if (u > maxNodeId) maxNodeId = u;
        if (v > maxNodeId) maxNodeId = v;
    }

    fEdges.clear(); // This should not be needed in C++11 but as it seems it actually is needed
    fEdges.seekg(0);

    vector<node> nodes(maxNodeId + 1);

    for (; getline(fEdges, line);) {
        sscanf(line.c_str(), "%d;%d;%d;", &id, &u, &v);

        if(nodes[u] == nullptr)
            nodes[u] = G.newNode(u);

        if(nodes[v] == nullptr)
            nodes[v] = G.newNode(v);

        G.newEdge(nodes[u], nodes[v], id);
    }
}

void graph2csv(const Graph &G, ostream &fGraph)
{
    edge e;
    forall_edges(e, G) {
        fGraph << e->index() << ";" << e->source() << ";" << e->target() << endl;
    }
}

void graph2dot(const Graph &G, ostream &fGraph)
{
    fGraph << "graph G {" << endl;

    edge e;
    forall_edges(e, G) {
        fGraph << "\t" << e->source() << " -- " << e->target() << " [label \"" << e->index() << "\"]" << endl;
    }

    fGraph << "}" << endl;
}

ostream & operator<<(ostream &os, const set<edge> &S)
{
    // TODO: Reduce to   operator<<(ostream &os, const std::set<int>& S)
    int i = 0, ss = S.size();
    for(auto e : S) {
        os << e->index();
        if (i < ss - 1) os << ",";
        i++;
    }
    return os;
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

ostream & operator<<(ostream &os, const List<edge> &L)
{
    int i = 0, ls = L.size();
    for(auto e : L) {
        os << e->index();
        if (i < ls - 1) os << ",";
        i++;
    }
    return os;
}

string nameColor(Color c)
{
    switch(c) {
        case Color::BLACK: return "black";
        case Color::RED: return "red";
        case Color::BLUE: return "blue";
        default: return "ERROR";
    }
}

string nodesByColor2str(const Graph &G, const GraphColoring &coloring, Color c)
{
    stringstream s;
    node n;

    int x = 0;
    forall_nodes(n, G) {
        if (coloring[n] == c) {
            x++;
        }
    }

    forall_nodes(n, G) {
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


string edgesByColor2str(const Graph &G, const GraphColoring &coloring, Color c)
{
    stringstream s;
    edge e;

    int x = 0;
    forall_edges(e, G) {
        if (coloring[e] == c) {
            x++;
        }
    }

    forall_edges(e, G) {
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

string coloring2str(const Graph &G, const GraphColoring &c)
{
    stringstream s;

    s << "Nodes: "
      << "(blue) " << nodesByColor2str(G, c, Color::BLUE) << "; "
      << "(red) " << nodesByColor2str(G, c, Color::RED) << "; "
      << "(black) " << nodesByColor2str(G, c, Color::BLACK) << "; ";

    s << "Edges: "
      << "(blue) " << edgesByColor2str(G, c, Color::BLUE) << "; "
      << "(red) " << edgesByColor2str(G, c, Color::RED) << "; "
      << "(black) " << edgesByColor2str(G, c, Color::BLACK) << "; ";

    return s.str();
}

string edgelist2str(const ogdf::List<edge> &edges)
{
    stringstream s;
    forall_listiterators(edge, it, edges) {
        s << (*it)->index();
        if (*it != edges.back()) {
            s << ", ";
        }
    }
    return s.str();
}

edge edgeByIndex(const List<edge> &edges, int index)
{
    for(edge e : edges) if(e->index() == index) return e;
    return nullptr;
}

void indicies2edges(const List<edge> &graphEdges, const string &str, List<edge> &l)
{
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
bool isCut(Graph &G, const List<edge> &cut)
{
    for (auto e : cut) {
        G.hideEdge(e);
    }

    bool r = isConnected(G);

    G.restoreAllEdges();
    return !r;
}

/**
 * Helper function todetermine whether given set of edges really is a minimal cut, w.r.t. # of components of G\cut
 * Returns 0 on success, -1 if # of components is 1. Otherwise, the # of components of G\cut
 */
int isMinCut(Graph &G, const List<edge> &cut, int &ncomponents)
{
    NodeArray<int> component(G);

    for(auto e : cut) {
        G.hideEdge(e);
    }
    ncomponents = connectedComponents(G, component);

    if (ncomponents == 1) {
        G.restoreAllEdges();
        return -1;
    }

    for(auto e : cut) {
        G.restoreEdge(e);

        // Count # of components of smaller cut
        int nSmallerCutComponents = connectedComponents(G, component);

        // If numbers of components is the same and it's still a cut then obviously the original cut was not minimal
        if(nSmallerCutComponents == ncomponents) {
            G.restoreAllEdges();
            return ncomponents;
        }
        G.hideEdge(e);
    }

    G.restoreAllEdges();

    return 0;
}

int isMinCut(Graph &G, const List<edge> &cut)
{
    int nc;
    return isMinCut(G, cut, nc);
}

void bruteforceGraphBonds(Graph &G, int cutSizeBound, int minComponents,
                int maxComponents, List<List<edge>> &bonds)
{
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
