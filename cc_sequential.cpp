// Algorithm of Sequential Connected Components using label propagation

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>

using namespace std;

// Copy CSRGraph struct + prototype from conversion_graph.cpp
struct CSRGraph {
    int n; // number of vertices
    vector<int> rowptr; // size: n+1
    vector<int> colind; // size: number of directed edges
};


CSRGraph load_mtx_as_csr(const string &filename);


// Function for sequential CC (label propagation)
void cc_sequential(const CSRGraph &g, vector<int> &label) {
    int n = g.n;

    label.resize(n);
    vector<int> new_label(n);

    // Initial label(each vertex has its own ID)
    for (int v = 0; v < n; v++) {
        label[v] = v;
    }

    bool changed = true;

    while (changed) {
        changed = false;

        for (int v = 0; v < n; v++) {
            int best = label[v];

            // Scan neighbors
            for (int e = g.rowptr[v]; e < g.rowptr[v+1]; e++) {
                int u = g.colind[e];
                if (label[u] < best) {
                    best = label[u];
                }
            }

            new_label[v] = best;
            if (best != label[v]) {
                changed = true;
            }
        }

        // Swap buffers
        label.swap(new_label);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "Usage: ./cc_sequential matrix.mtx" << endl;
        return 1;
    }

    string filename = argv[1];

    //Load graph
    CSRGraph G = load_mtx_as_csr(filename);

    //Print number of vertices
    cout << "Graph has " << G.n << " vertices." << endl;
    
    //Compute connected components
    vector<int> label;
    cc_sequential(G, label);

    //Count distinct labels
    vector<int> tmp = label;
    sort(tmp.begin(), tmp.end());
    tmp.erase(unique(tmp.begin(), tmp.end()), tmp.end());

    cout << "Number of connected components: " << tmp.size() << endl;

    return 0;
}