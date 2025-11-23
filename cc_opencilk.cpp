#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

using namespace std;

/*Copy CSRGraph struct + prototype from conversion_graph.cpp*/
struct CSRGraph {
    int n;
    vector<int> rowptr;
    vector<int> colind;
};

CSRGraph load_mtx_as_csr(const string &filename);

void cc_opencilk(const CSRGraph &g, vector<int> &label) {
    int n = g.n;
    label.resize(n);
    vector<int> new_label(n);
    
    /*Initial label(each vertex has its own ID)*/
    cilk_for (int v = 0; v < n; v++) {
        label[v] = v;
    }
    
    bool changed = true;
    int iterations = 0;
    
    while (changed) {
        changed = false;
        iterations++;
        
        /*Atomic variable to track if any label changed*/
        volatile int any_changed = 0;
        
        /*Parallelize the per-vertex loop*/
        cilk_for (int v = 0; v < n; v++) {
            int best = label[v];
            
            /*Scans neighbors*/
            for (int e = g.rowptr[v]; e < g.rowptr[v+1]; e++) {
                int u = g.colind[e];
                if (label[u] < best) {
                    best = label[u];
                }
            }
            
            new_label[v] = best;
            
            if (best != label[v]) {
                /*Uses atomic operation to set flag*/
                __sync_fetch_and_or(&any_changed, 1);
            }
        }
        
        /*Checks if any changes occurred*/
        changed = (any_changed != 0);
        
        /*Swaps buffers*/
        label.swap(new_label);
    }
    
    cout << "OpenCilk algorithm converged in " << iterations << " iterations" << endl;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "Usage: ./cc_opencilk matrix.mtx [num_workers]" << endl;
        return 1;
    }
    
    string filename = argv[1];
    
   /* Set number of workers if provided:
    if (argc >= 3) {
        int num_workers = atoi(argv[2]);
        if (num_workers > 0) {
            __cilkrts_set_param("nworkers", argv[2]);
        }
    } */

    cout << "Using " << __cilkrts_get_nworkers() << " OpenCilk workers" << endl;
    
    // 1)Load graph
    CSRGraph G = load_mtx_as_csr(filename);
    cout << "Graph loaded: " << G.n << " vertices, " 
         << (G.rowptr.empty() ? 0 : G.rowptr.back()) << " edges" << endl;
    
    // 2)Compute connected components
    vector<int> label;
    cc_opencilk(G, label);
    
    // 3)Count distinct labels (connected components)
    vector<int> tmp = label;
    sort(tmp.begin(), tmp.end());
    tmp.erase(unique(tmp.begin(), tmp.end()), tmp.end());
    
    cout << "Number of connected components: " << tmp.size() << endl;
    
    return 0;
}