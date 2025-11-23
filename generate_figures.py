import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import sys

# CSV format: graph,vertices,edges,components,impl,threads,time,iterations

def load_data(csv_file):
    """Load performance data from CSV file"""
    try:
        df = pd.read_csv(csv_file)
        return df
    except FileNotFoundError:
        print(f"Error: Could not find {csv_file}")
        sys.exit(1)

def figure1_speedup_vs_threads(df, graph_name=None):
    """
    Figure 1: Speedup vs. Number of Threads
    """
    if graph_name:
        data = df[df['graph'] == graph_name]
    else:
        data = df[df['graph'] == df['graph'].iloc[0]]

    seq_time = data[data['impl'] == 'sequential']['time'].iloc[0]
    implementations = ['openmp', 'opencilk', 'pthreads']

    plt.figure(figsize=(10, 6))

    for impl in implementations:
        impl_data = data[data['impl'] == impl].sort_values('threads')
        if len(impl_data) > 0:
            threads = impl_data['threads'].values
            times = impl_data['time'].values
            speedup = seq_time / times
            plt.plot(threads, speedup, 'o-', label=impl.upper(), linewidth=2, markersize=8)

    # Ideal line
    max_threads = data['threads'].max()
    ideal = np.arange(1, max_threads + 1)
    plt.plot(ideal, ideal, 'k--', label='Ideal', linewidth=2)

    plt.xlabel('Number of Threads')
    plt.ylabel('Speedup')
    plt.title(f'Speedup vs Thread Count (Graph: {graph_name or "default"})')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig('figure1_speedup_vs_threads.png', dpi=300)
    print("Saved: figure1_speedup_vs_threads.png")


def figure2_time_vs_threads(df, graph_name=None):
    """
    Figure 2: Execution Time vs Number of Threads
    """
    if graph_name:
        data = df[df['graph'] == graph_name]
    else:
        data = df[df['graph'] == df['graph'].iloc[0]]

    plt.figure(figsize=(10, 6))

    implementations = ['openmp', 'opencilk', 'pthreads']
    for impl in implementations:
        impl_data = data[data['impl'] == impl].sort_values('threads')
        if len(impl_data) > 0:
            threads = impl_data['threads'].values
            times = impl_data['time'].values
            plt.plot(threads, times, 'o-', label=impl.upper(), linewidth=2, markersize=8)

    plt.xlabel('Number of Threads')
    plt.ylabel('Execution Time (seconds)')
    plt.title(f'Execution Time vs Thread Count (Graph: {graph_name or "default"})')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig('figure2_time_vs_threads.png', dpi=300)
    print("Saved: figure2_time_vs_threads.png")


def figure3_efficiency_vs_threads(df, graph_name=None):
    """
    Figure 3: Efficiency vs. Thread Count
    """
    if graph_name:
        data = df[df['graph'] == graph_name]
    else:
        data = df[df['graph'] == df['graph'].iloc[0]]

    seq_time = data[data['impl'] == 'sequential']['time'].iloc[0]
    implementations = ['openmp', 'opencilk', 'pthreads']

    plt.figure(figsize=(10, 6))

    for impl in implementations:
        impl_data = data[data['impl'] == impl].sort_values('threads')
        if len(impl_data) > 0:
            threads = impl_data['threads'].values
            times = impl_data['time'].values
            speedup = seq_time / times
            efficiency = speedup / threads
            plt.plot(threads, efficiency, 'o-', label=impl.upper(), linewidth=2, markersize=8)

    plt.axhline(y=1.0, color='k', linestyle='--', linewidth=2, label='Perfect Efficiency')

    plt.xlabel('Number of Threads')
    plt.ylabel('Efficiency (Speedup/Threads)')
    plt.title(f'Efficiency vs Thread Count (Graph: {graph_name or "default"})')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.ylim(0, 1.2)
    plt.tight_layout()
    plt.savefig('figure3_efficiency_vs_threads.png', dpi=300)
    print("Saved: figure3_efficiency_vs_threads.png")


def figure4_comparison_bar_chart(df):
    """
    Figure 4: Comparison Bar Chart
    """
    plt.figure(figsize=(12, 6))

    graphs = df['graph'].unique()
    implementations = ['sequential', 'openmp', 'opencilk', 'pthreads']
    x = np.arange(len(graphs))
    width = 0.2

    for i, impl in enumerate(implementations):
        times = []
        for graph in graphs:
            graph_data = df[df['graph'] == graph]
            impl_data = graph_data[graph_data['impl'] == impl]
            if impl != 'sequential':
                impl_data = impl_data[impl_data['threads'] == 4]

            times.append(impl_data['time'].iloc[0] if len(impl_data) else 0)

        plt.bar(x + i*width, times, width, label=impl.upper(), alpha=0.8)

    plt.xlabel('Test Graph')
    plt.ylabel('Execution Time (seconds)')
    plt.title('Performance Comparison Across Test Cases')
    plt.xticks(x + width * 1.5, graphs, rotation=45, ha='right')
    plt.legend()
    plt.grid(True, alpha=0.3, axis='y')
    plt.tight_layout()
    plt.savefig('figure4_comparison_bar_chart.png', dpi=300)
    print("Saved: figure4_comparison_bar_chart.png")


def main():
    if len(sys.argv) < 2:
        print("Usage: python generate_figures.py <performance_data.csv>")
        sys.exit(1)

    csv_file = sys.argv[1]
    df = load_data(csv_file)

    print(f"Loaded {len(df)} data points")
    print("\nGenerating figures...")

    figure1_speedup_vs_threads(df)
    figure2_time_vs_threads(df)
    figure3_efficiency_vs_threads(df)
    figure4_comparison_bar_chart(df)

    print("\nAll figures generated!")


if __name__ == "__main__":
    main()
