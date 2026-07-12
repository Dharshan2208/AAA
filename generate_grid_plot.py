#!/usr/bin/env python3
import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Set clean aesthetic style for matplotlib
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
plt.rcParams.update({
    'font.size': 11,
    'axes.labelsize': 12,
    'axes.titlesize': 13,
    'xtick.labelsize': 10,
    'ytick.labelsize': 10,
    'figure.titlesize': 15,
    'font.family': 'sans-serif',
    'grid.alpha': 0.3,
    'grid.linestyle': '--'
})

# Color palette and markers for algorithms
ALGO_STYLE = {
    'Naive': {'color': '#d95f02', 'marker': 'o'},     # Orange
    'Heap': {'color': '#7570b3', 'marker': 's'},      # Purple
    'Set': {'color': '#1b9e77', 'marker': '^'},       # Green
    'LRU': {'color': '#377eb8', 'marker': 'D'},       # Blue
    'FIFO': {'color': '#e7298a', 'marker': 'v'}       # Pink
}

def main():
    csv_path = 'results/benchmark_results.csv'
    if not os.path.exists(csv_path):
        print(f"Error: {csv_path} not found.")
        return

    plot_dir = 'results/plots'
    os.makedirs(plot_dir, exist_ok=True)

    # Load and clean data
    df = pd.read_csv(csv_path)
    df['HitRatio'] = df['Hits'] / (df['Hits'] + df['Misses'])
    grouped = df.groupby(['Implementation', 'CacheSize', 'Requests']).mean(numeric_only=True).reset_index()

    # Filter data
    target_caches = [50, 100]
    target_requests = [100, 1000, 10000]
    
    sub_df = grouped[grouped['CacheSize'].isin(target_caches) & grouped['Requests'].isin(target_requests)]
    if sub_df.empty:
        print("Error: No data matching CacheSize in [50, 100] and Requests in [100, 1000, 10000].")
        return

    # We will construct a side-by-side plot
    fig, axes = plt.subplots(1, 2, figsize=(16, 7), sharey=False)

    for i, cache_size in enumerate(target_caches):
        ax1 = axes[i]
        cache_df = sub_df[sub_df['CacheSize'] == cache_size]
        
        # Create twin axis for execution time
        ax2 = ax1.twinx()
        
        # We will plot on categorical x-ticks for request counts [100, 1000, 10000]
        x_labels = [f"{req:,}" for req in target_requests]
        x_indices = np.arange(len(target_requests))
        
        # Sort request values to map them correctly
        for algo in ['FIFO', 'LRU', 'Naive', 'Heap', 'Set']:
            algo_df = cache_df[cache_df['Implementation'] == algo].sort_values('Requests')
            if algo_df.empty:
                continue
                
            # Filter to match target request counts exactly
            algo_df = algo_df[algo_df['Requests'].isin(target_requests)]
            
            # Map request counts to indices
            y_hits = [algo_df[algo_df['Requests'] == r]['HitRatio'].values[0] * 100 for r in target_requests]
            y_time = [algo_df[algo_df['Requests'] == r]['RuntimeMicroseconds'].values[0] for r in target_requests]
            
            # Plot Hit Ratio (solid line, left Y-axis)
            ax1.plot(x_indices, y_hits, 
                     linestyle='-', 
                     linewidth=2.5, 
                     color=ALGO_STYLE[algo]['color'], 
                     marker=ALGO_STYLE[algo]['marker'], 
                     markersize=8, 
                     label=f"{algo} Hit Ratio")
            
            # Plot Execution Time (dashed line, right Y-axis)
            ax2.plot(x_indices, y_time, 
                     linestyle='--', 
                     linewidth=1.8, 
                     color=ALGO_STYLE[algo]['color'], 
                     marker=ALGO_STYLE[algo]['marker'], 
                     markersize=6, 
                     markerfacecolor='none', 
                     label=f"{algo} Runtime")

        # Set title and labels
        ax1.set_title(f"Cache Capacity K = {cache_size}", pad=12, fontweight='semibold')
        ax1.set_xlabel("Request Sequence Length (Total Requests)", labelpad=10)
        ax1.set_ylabel("Hit Ratio (%)", labelpad=8)
        ax1.set_xticks(x_indices)
        ax1.set_xticklabels(x_labels)
        ax1.set_ylim(-5, 105)
        
        ax2.set_ylabel("Avg Execution Time (Microseconds)", labelpad=8)
        
        # Align grids nicely
        ax1.grid(True, which='both', axis='both', linestyle='--', alpha=0.3)
        ax2.grid(False) # Turn off right grid to avoid clutter

        # Store legend handles from the first plot to construct a combined legend
        if i == 0:
            lines1, labels1 = ax1.get_legend_handles_labels()
            lines2, labels2 = ax2.get_legend_handles_labels()
    
    # Filter to get unique entries for the legend
    # Left axis legends are solid lines (Hit Ratio), right axis are dashed (Runtimes)
    legend_handles = []
    legend_labels = []
    
    # We want to display them nicely grouped
    for l, label in zip(lines1, labels1):
        if label not in legend_labels:
            legend_handles.append(l)
            legend_labels.append(label)
    for l, label in zip(lines2, labels2):
        if label not in legend_labels:
            legend_handles.append(l)
            legend_labels.append(label)
            
    # Put legend below the plots
    fig.legend(legend_handles, legend_labels, 
               loc='lower center', 
               ncol=5, 
               bbox_to_anchor=(0.5, 0.01), 
               frameon=True, 
               facecolor='white', 
               edgecolor='none')

    plt.suptitle("Algorithm Performance Analysis: Hit Ratio & Execution Time Comparison", y=0.96, fontweight='bold')
    plt.subplots_adjust(bottom=0.18, wspace=0.35)
    
    # Save plot
    out_path = os.path.join(plot_dir, 'cache_50_100_comparison.png')
    plt.savefig(out_path, dpi=300)
    plt.close()
    print(f"Generated: {out_path}")

if __name__ == "__main__":
    main()
