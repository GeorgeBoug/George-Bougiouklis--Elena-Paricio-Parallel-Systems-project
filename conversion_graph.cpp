// This code loads a Matrix Market (.mtx) file and converts it to CSR (Compressed Sparse Row) format

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdlib>

using namespace std;

// CSR graph structure 
struct CSRGraph {
    int n;                 // number of vertices
    vector<int> rowptr;    // size n+1 (int* rowptr)
    vector<int> colind;    // adjacency list (all neighbors) size = number of directed edges (int* col_ind)
};


// Function to convert .mtx into CSR format
CSRGraph load_mtx_as_csr(const string &filename) {
    ifstream fin(filename.c_str());
    if (!fin) {
        cerr << "Error opening file " << filename << endl;
        exit(1);
    }

    string line;

    // Skip comment lines starting with '%'
    do {
        if (!getline(fin, line)) {
            cerr << "Error: file ended before header" << endl;
            exit(1);
        }
    } while (!line.empty() && line[0] == '%');

    // Parse header line: nrows ncols nnz
    int nrows, ncols, nnz;
    {
        stringstream ss(line);
        ss >> nrows >> ncols >> nnz;
    }

    if (nrows != ncols) {
        cerr << "Matrix must be square to represent a graph." << endl;
        exit(1);
    }

    int n = nrows;

    // Read COO entries and symmetrize
    vector<pair<int,int> > edges;
    edges.reserve(2 * nnz);

    int i, j;
    while (fin >> i >> j) {
        i--; j--;            // convert to 0-based indexing

        if (i == j) continue;  // ignore self-loops

        edges.push_back( make_pair(i, j) );
        edges.push_back( make_pair(j, i) );  // symmetric
    }

    int M = edges.size();

    // Count neighbors per vertex
    vector<int> rowcount(n, 0);
    for (int k = 0; k < M; k++) {
        int v = edges[k].first;
        rowcount[v]++;
    }

    // Build rowptr with prefix sum
    CSRGraph G;
    G.n = n;
    G.rowptr.resize(n + 1);
    G.rowptr[0] = 0;

    for (int v = 0; v < n; v++) {
        G.rowptr[v+1] = G.rowptr[v] + rowcount[v];
    }

    // Allocate colind
    G.colind.resize(M);
    vector<int> cur_pos(n);

    for (int v = 0; v < n; v++) {
        cur_pos[v] = G.rowptr[v];
    }

    // Fill adjacency lists
    for (int k = 0; k < M; k++) {
        int v = edges[k].first;
        int u = edges[k].second;

        int p = cur_pos[v]++;
        G.colind[p] = u;
    }

    return G;
}