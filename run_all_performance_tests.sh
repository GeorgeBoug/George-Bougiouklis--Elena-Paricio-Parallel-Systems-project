#!/usr/bin/env bash

###############################################################################
# Connected Components – Full Performance Testing Pipeline (Linux Bash Version)
###############################################################################

# Defaults
GRAPH_FILES=("com-Orkut.mtx")
THREADS=(1 2 4 8 16)
OUTPUT_CSV="performance_data.csv"
SKIP_BUILD=0

###############################################################################
# Parse Command-Line Arguments
###############################################################################

while [[ $# -gt 0 ]]; do
    case $1 in
        -g|--graphs)
            IFS=',' read -ra GRAPH_FILES <<< "$2"
            shift 2
            ;;
        -t|--threads)
            IFS=',' read -ra THREADS <<< "$2"
            shift 2
            ;;
        -o|--output)
            OUTPUT_CSV="$2"
            shift 2
            ;;
        --skip-build)
            SKIP_BUILD=1
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

###############################################################################
# STEP 1 – Verify Executables
###############################################################################

if [[ $SKIP_BUILD -eq 0 ]]; then
    echo "Checking executables..."

    REQUIRED_EXE=("cc_sequential" "cc_openmp" "cc_pthreads" "cc_opencilk")
    MISSING=()

    for exe in "${REQUIRED_EXE[@]}"; do
        if [[ ! -f "$exe" ]]; then
            MISSING+=("$exe")
        fi
    done

    if (( ${#MISSING[@]} > 0 )); then
        echo "Missing executables: ${MISSING[*]}"
        echo ""
        echo "Compile them using:"
        echo "  g++ -O3 -std=c++17 new_seq.cpp conversion_graph.cpp -o cc_sequential"
        echo "  g++ -O3 -std=c++17 -fopenmp new_openmp.cpp conversion_graph.cpp -o cc_openmp"
        echo "  g++ -O3 -std=c++17 -pthread new_pthreads.cpp conversion_graph.cpp -o cc_pthreads"
        echo "  clang++ -O3 -std=c++17 -fopencilk new_opencilk.cpp conversion_graph.cpp -o cc_opencilk"
        exit 1
    fi

    echo "All executables found."
fi

###############################################################################
# STEP 2 – Validate Graph Files
###############################################################################

echo "Validating graph files..."

VALID_GRAPHS=()

for g in "${GRAPH_FILES[@]}"; do
    if [[ -f "$g" ]]; then
        echo "  Found: $g"
        VALID_GRAPHS+=("$g")
    else
        echo "  Warning: $g not found, skipping."
    fi
done

if (( ${#VALID_GRAPHS[@]} == 0 )); then
    echo "ERROR: No valid graph files found!"
    exit 1
fi

###############################################################################
# STEP 3 – Run Tests
###############################################################################

echo "Running performance tests..."
echo "Graphs: ${VALID_GRAPHS[*]}"
echo "Threads: ${THREADS[*]}"

# Initialize CSV
echo "graph,vertices,edges,components,impl,threads,time,iterations" > "$OUTPUT_CSV"


###############################################################################
# Function: run_test (exec, args..., impl, threads)
###############################################################################

run_test() {
    local executable="$1"
    shift
    local graph="$1"
    local impl="$2"
    local threads="$3"
    shift 2

    if [[ ! -f "$executable" ]]; then
        return
    fi

    echo "  Running: $executable on $graph ($impl, $threads threads)"

    # Start timer
    start=$(date +%s.%N)

    # Capture output
    if [[ "$impl" == "opencilk" ]]; then
        export CILK_NWORKERS="$threads"
        output=$(./"$executable" "$graph" 2>&1)
    else
        output=$(./"$executable" "$graph" "$threads" 2>&1)
    fi

    end=$(date +%s.%N)
    elapsed=$(echo "$end - $start" | bc)

    # Metrics
    vertices=$(echo "$output" | grep -Eo "vertices[: ]+[0-9]+" | grep -Eo "[0-9]+" | head -1)
    edges=$(echo "$output"     | grep -Eo "[0-9]+ edges"       | grep -Eo "[0-9]+" | head -1)
    components=$(echo "$output" | grep -Eo "connected components[: ]+[0-9]+" | grep -Eo "[0-9]+" | head -1)
    iterations=$(echo "$output" | grep -Eo "converged in [0-9]+ iterations" | grep -Eo "[0-9]+" | head -1)

    [[ -z "$iterations" ]] && iterations=0
    [[ -z "$vertices" ]]   && vertices=0
    [[ -z "$edges" ]]      && edges=0
    [[ -z "$components" ]] && components=0

    # Append to CSV
    echo "$(basename "$graph"),$vertices,$edges,$components,$impl,$threads,$elapsed,$iterations" >> "$OUTPUT_CSV"
}


###############################################################################
# Main Loop
###############################################################################

for graph in "${VALID_GRAPHS[@]}"; do
    echo "Testing graph: $graph"

    # Sequential baseline
    run_test "cc_sequential" "$graph" "sequential" 1

    # OpenMP
    for t in "${THREADS[@]}"; do
        run_test "cc_openmp" "$graph" "openmp" "$t"
    done

    # OpenCilk
    for t in "${THREADS[@]}"; do
        run_test "cc_opencilk" "$graph" "opencilk" "$t"
    done

    # Pthreads
    for t in "${THREADS[@]}"; do
        run_test "cc_pthreads" "$graph" "pthreads" "$t"
    done

    echo ""
done

###############################################################################
# STEP 4 – Run Visualization (Python)
###############################################################################

echo "Running Python figure generation..."

if [[ ! -f generate_figures.py ]]; then
    echo "ERROR: generate_figures.py not found!"
else
    python3 generate_figures.py "$OUTPUT_CSV"
fi

echo ""
echo "==============================================="
echo "   PERFORMANCE TESTING COMPLETE (Linux Bash)   "
echo "==============================================="
echo "CSV Saved to: $OUTPUT_CSV"
echo ""

