#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <omp.h>
using namespace std;

/*Copy CSRGraph struct + prototype from conversion_graph.cpp*/
struct CSRGraph {
    int n;
    vector<int> rowptr;
    vector<int> colind;
};

CSRGraph load_mtx_as_csr(const string &filename);

void cc_openmp(const CSRGraph &g, vector<int> &label) {
    int n = g.n;
    label.resize(n);
    vector<int> new_label(n);
    

    #pragma omp parallel for
    for (int v = 0; v < n; v++) {
        label[v] = v;
    }
    
    bool changed = true;
    int iterations = 0;
    
    while (changed) {
        changed = false;
        iterations++;
        
        /*Uses reduction to track if any label changed*/
        int any_changed = 0;
        
        #pragma omp parallel
        {
            int thread_changed = 0;
            
            /*Parallelizes the per-vertex loop*/
            #pragma omp for
            for (int v = 0; v < n; v++) {
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
                    thread_changed = 1;
                }
            }
            
            /*Reduction to check if any thread made changes*/
            #pragma omp atomic
            any_changed |= thread_changed;
        }
        
        changed = (any_changed != 0);
        
        /*Swap buffers*/
        label.swap(new_label);
    }
    
    cout << "OpenMP algorithm converged in " << iterations << " iterations" << endl;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "Usage: ./cc_openmp matrix.mtx [num_threads]" << endl;
        return 1;
    }
    
    string filename = argv[1];
    
    /*Set number of threads if provided*/
    if (argc >= 3) {
        int num_threads = atoi(argv[2]);
        if (num_threads > 0) {
            omp_set_num_threads(num_threads);
        }
    }
    
    cout << "Using " << omp_get_max_threads() << " OpenMP threads" << endl;
    
    // 1)Load graph
    CSRGraph G = load_mtx_as_csr(filename);
    cout << "Graph loaded: " << G.n << " vertices, " 
         << (G.rowptr.empty() ? 0 : G.rowptr.back()) << " edges" << endl;
    
    // 2)Compute connected components
    vector<int> label;
    cc_openmp(G, label);
    
    // 3)Count distinct labels (connected components)
    vector<int> tmp = label;
    sort(tmp.begin(), tmp.end());
    tmp.erase(unique(tmp.begin(), tmp.end()), tmp.end());
    
    cout << "Number of connected components: " << tmp.size() << endl;
    
    return 0;
}