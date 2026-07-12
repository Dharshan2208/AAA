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
    'figure.titlesize': 14,
    'font.family': 'sans-serif',
    'grid.alpha': 0.3,
    'grid.linestyle': '--'
})

def main():
    csv_path = 'results/benchmark_results.csv'
    if not os.path.exists(csv_path):
        print(f"Error: {csv_path} not found. Run the benchmark executable first.")
        return

    # Load and clean data
    df = pd.read_csv(csv_path)
    
    # Calculate Hit Ratio from Hits / (Hits + Misses)
    df['HitRatio'] = df['Hits'] / (df['Hits'] + df['Misses'])
    
    # Aggregate data by taking the mean for duplicates (if any)
    grouped = df.groupby(['Implementation', 'CacheSize', 'Requests']).mean(numeric_only=True).reset_index()

    plot_dir = 'results/plots'
    os.makedirs(plot_dir, exist_ok=True)

    # We will target a specific configuration: Cache Size = 10, Requests = 10,000
    target_cache = 10
    target_requests = 10000
    
    sub_df = grouped[(grouped['CacheSize'] == target_cache) & 
                     (grouped['Requests'] == target_requests)]
    
    if sub_df.empty:
        # Fallback to any available large request size if 10,000 requests is not found
        available_requests = grouped['Requests'].unique()
        if len(available_requests) > 0:
            target_requests = sorted(available_requests)[-1]
            sub_df = grouped[(grouped['CacheSize'] == target_cache) & 
                             (grouped['Requests'] == target_requests)]
            
    if sub_df.empty:
        print("Error: No data found to generate the combined plot.")
        return

    # Define implementations and ordering
    ordered_impls = ['FIFO', 'LRU', 'Naive', 'Heap', 'Set']
    
    runtimes = []
    hit_ratios = []
    
    for impl in ordered_impls:
        row = sub_df[sub_df['Implementation'] == impl]
        if not row.empty:
            runtimes.append(row['RuntimeMicroseconds'].values[0])
            hit_ratios.append(row['HitRatio'].values[0] * 100)
        else:
            runtimes.append(0)
            hit_ratios.append(0)

    fig, ax1 = plt.subplots(figsize=(9, 5.5))

    # --- Left Y-Axis: Execution Time (Bar Chart) ---
    bar_width = 0.4
    x_indices = np.arange(len(ordered_impls))
    
    # Elegant blue shade for execution time
    color_time = '#4682B4' 
    bars = ax1.bar(x_indices, runtimes, width=bar_width, color=color_time, alpha=0.85, label='Execution Time (us)')
    
    ax1.set_xlabel('Cache Implementation', labelpad=10)
    ax1.set_ylabel('Avg Execution Time (Microseconds)', color=color_time, labelpad=10)
    ax1.tick_params(axis='y', labelcolor=color_time)
    ax1.set_xticks(x_indices)
    ax1.set_xticklabels(ordered_impls)
    
    # Add labels on top of the bars
    for bar in bars:
        height = bar.get_height()
        if height > 0:
            ax1.text(bar.get_x() + bar.get_width()/2.0, height + (max(runtimes) * 0.015),
                     f"{height:.1f} us", ha='center', va='bottom', color='#2b2b2b', fontsize=9.5)
            
    # Adjust y-limit to give room for labels
    ax1.set_ylim(0, max(runtimes) * 1.12)
    ax1.grid(True, which='both', axis='y', linestyle='--', alpha=0.3)

    # --- Right Y-Axis: Hit Ratio (Line Chart) ---
    ax2 = ax1.twinx()
    
    # Elegant orange-red color for hit ratio
    color_hit = '#E67E22' 
    line = ax2.plot(x_indices, hit_ratios, color=color_hit, marker='o', linewidth=3, markersize=8, 
                    label='Hit Ratio (%)')
    
    ax2.set_ylabel('Hit Ratio (%)', color=color_hit, labelpad=10)
    ax2.tick_params(axis='y', labelcolor=color_hit)
    ax2.set_ylim(-5, 105)
    
    # Add values next to the line points
    for idx, hit in enumerate(hit_ratios):
        if hit > 0:
            ax2.text(idx, hit + 3 if hit < 95 else hit - 6, f"{hit:.2f}%", 
                     ha='center', va='bottom', color=color_hit, fontweight='bold', fontsize=9.5)
            
    # Clear grid on right axis to avoid visual overlap
    ax2.grid(False)

    # Combine legends from both axes
    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, loc='upper left', frameon=True, facecolor='white', edgecolor='none')

    plt.title(f"Combined Evaluation: Execution Time vs. Hit Ratio\n(Cache Size = {target_cache}, Requests = {target_requests:,})", pad=15)
    plt.tight_layout()

    out_path = os.path.join(plot_dir, 'combined_performance.png')
    plt.savefig(out_path, dpi=300)
    plt.close()
    print(f"Generated: {out_path}")

if __name__ == "__main__":
    main()
