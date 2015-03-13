#include "helpers.h"

/**
 * Reads a csv file with lines "<id>;<source>;<target>;..." and transforms it into a graph
 */
void csvToGraph(ogdf::Graph &G, std::ifstream &fEdges) {
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

    std::vector<node> nodes(maxNodeId + 1);

    for (; getline(fEdges, line);) {
        sscanf(line.c_str(), "%d;%d;%d;", &id, &u, &v);

        if(nodes[u] == nullptr)
            nodes[u] = G.newNode(u);

        if(nodes[v] == nullptr)
            nodes[v] = G.newNode(v);

        G.newEdge(nodes[u], nodes[v], id);
    }
}

std::ostream & operator<<(std::ostream &os, const std::set<edge> &S){
    int i = 0, ss = S.size();
    for(auto e : S) {
        os << e->index();
        if (i < ss - 1) os << ",";
        i++;
    }
    return os;
}

std::ostream & operator<<(std::ostream &os, const ogdf::List<edge> &L)
{
    int i = 0, ls = L.size();
    for(auto e : L) {
        os << e->index();
        if (i < ls - 1) os << ",";
        i++;
    }
    return os;
}

std::ostream & operator<<(std::ostream &os, const ogdf::Graph &G)
{
    edge e;
    forall_edges(e, G) {
        os << "(" << e->source() << "," << e->target() << "), ";
    }
    return os;
}

string nameColor(Color c) {
    switch(c) {
        case Color::BLACK: return "black";
        case Color::RED: return "red";
        case Color::BLUE: return "blue";
        default: return "ERROR";
    }
}

string coloring2str(const ogdf::Graph &G, const GraphColoring &c)
{
    std::stringstream s;

    node n;

    s << "Nodes: ";
    forall_nodes(n, G) {
        s << n->index() << " " << nameColor(c[n]) << "; ";
    }
    s << endl;

    edge e;

    s << "Edges: ";
    forall_edges(e, G) {
        s << e->index() << " " << nameColor(c[e]) << "; ";
    }
    s << endl;

    return s.str();
}

edge edgeByIndex(const ogdf::List<edge> &edges, int index)
{
    for(edge e : edges) if(e->index() == index) return e;
    return nullptr;
}

void indicies2edges(const ogdf::List<edge> &graphEdges, const string &str, ogdf::List<edge> &l)
{
    std::stringstream ss(str);
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
bool isCut(ogdf::Graph &G, const ogdf::List<edge> &cut)
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
int isMinCut(ogdf::Graph &G, const ogdf::List<edge> &cut, int &ncomponents)
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

int isMinCut(ogdf::Graph &G, const ogdf::List<edge> &cut) {
    int nc;
    return isMinCut(G, cut, nc);
}
