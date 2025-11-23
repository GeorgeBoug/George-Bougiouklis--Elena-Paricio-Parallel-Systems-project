//We have implementeed the threads algorithm through a prodcuer-consumer mechanism
//The main thread produces work by dividing vertices into chunks and each worker thread consumes one chunk and analyzes it 

#include <iostream>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include <cstdlib> // for atoi(conversion to integer)

using namespace std;

struct CSRGraph {
    int n;
    vector<int> rowptr;
    vector<int> colind;
};

// Implemented in conversion_graph.cpp
CSRGraph load_mtx_as_csr(const string &filename);

// Thread information
struct ThreadArgs {
    const CSRGraph *G; //pointer to graph
    const int *label_old;
    int *label_new;
    int start;
    int end;
    bool changed;
};

// Worker function: one thread processes vertices in start-end
void *worker(void *arg) {
    ThreadArgs *a = (ThreadArgs *)arg;
    const CSRGraph &G = *(a->G);
    const int *label_old = a->label_old;
    int *label_new = a->label_new;

    a->changed = false;

    for (int v = a->start; v < a->end; ++v) {
        int lv = label_old[v];

        int row_start = G.rowptr[v];
        int row_end   = G.rowptr[v + 1];

        // Look at neighbors using label_old only
        for (int idx = row_start; idx < row_end; ++idx) {
            int u = G.colind[idx];
            int lu = label_old[u];
            if (lu < lv) {
                lv = lu;
            }
        }

        label_new[v] = lv;          // write into label_new
        if (lv != label_old[v]) {   // if the label changed
            a->changed = true;
        }
    }

    return NULL;
}


// Parallel connected components with pthreads
void cc_pthreads(const CSRGraph &G, vector<int> &label, int num_threads) {
    int n = G.n;

    // label will hold the final labels
    label.resize(n);
    for (int v = 0; v < n; ++v) {
        label[v] = v;
    }

    // auxiliary array for new labels
    vector<int> new_label(n);

    bool changed = true;

    while (changed) {
        changed = false;

        const int *label_old = label.data();
        int *label_new = new_label.data();

        // Partition vertices into chunks for each thread
        int chunk = (n + num_threads - 1) / num_threads;

        vector<ThreadArgs> args(num_threads);
        vector<pthread_t> threads(num_threads);
        vector<bool> created(num_threads, false);

        for (int t = 0; t < num_threads; ++t) {
            int start = t * chunk;
            int end   = start + chunk;
            if (start > n) start = n;
            if (end   > n) end   = n;

            if (start >= end) {
                // this thread would have no work
                continue;
            }

            args[t].G         = &G;
            args[t].label_old = label_old;
            args[t].label_new = label_new;
            args[t].start     = start;
            args[t].end       = end;
            args[t].changed   = false;

            pthread_create(&threads[t], NULL, worker, &args[t]);
            created[t] = true;
        }

        // Join threads and collect "changed" flags
        for (int t = 0; t < num_threads; ++t) {
            if (!created[t]) continue;

            pthread_join(threads[t], NULL); // wait for this worker

            if (args[t].changed) {
                changed = true;
            }
        }

        // Swap old and new labels for the next iteration
        label.swap(new_label);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " matrix.mtx num_threads" << endl;
        return 1;
    }

    string filename = argv[1];
    int num_threads = atoi(argv[2]);
    if (num_threads <= 0) {
        cerr << "num_threads must be > 0" << endl;
        return 1;
    }

    // 1) Load graph
    CSRGraph G = load_mtx_as_csr(filename);
    cout << "Graph has " << G.n << " vertices." << endl;

    // 2) Compute CC with pthreads
    vector<int> label;
    cc_pthreads(G, label, num_threads);

    // 3) Count distinct labels (connected components)
    vector<int> tmp = label;
    sort(tmp.begin(), tmp.end());
    tmp.erase(unique(tmp.begin(), tmp.end()), tmp.end());

    cout << "Number of connected components: " << tmp.size() << endl;

    return 0;
}