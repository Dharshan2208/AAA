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
    'grid.alpha': 0.4,
    'grid.linestyle': '--'
})

# Curated harmonious color palette
COLORS = {
    'Naive': '#d95f02',       # Orange-Red
    'Heap': '#7570b3',        # Muted Purple
    'Set': '#1b9e77',         # Emerald Green
    'Belady': '#e7298a',      # Deep Pink (Optimal reference)
    'LRU': '#377eb8',         # Soft Blue
    'FIFO': '#4daf4a'         # Soft Green
}

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

    # -------------------------------------------------------------
    # Plot 1: Hit Ratio vs. Cache Size (for a fixed large request size, e.g. 10,000)
    # -------------------------------------------------------------
    target_requests = 10000
    sub_df = grouped[grouped['Requests'] == target_requests]
    
    if not sub_df.empty:
        plt.figure(figsize=(8, 5))
        
        # We only need one curve for Belady because Naive, Heap, and Set have identical hit ratios
        belady_data = sub_df[sub_df['Implementation'] == 'Heap'].sort_values('CacheSize')
        lru_data = sub_df[sub_df['Implementation'] == 'LRU'].sort_values('CacheSize')
        fifo_data = sub_df[sub_df['Implementation'] == 'FIFO'].sort_values('CacheSize')
        
        if not belady_data.empty:
            plt.plot(belady_data['CacheSize'], belady_data['HitRatio'] * 100, 
                     marker='o', linewidth=2.5, color=COLORS['Belady'], label='Belady (Optimal Offline)')
        if not lru_data.empty:
            plt.plot(lru_data['CacheSize'], lru_data['HitRatio'] * 100, 
                     marker='s', linewidth=2, color=COLORS['LRU'], label='LRU')
        if not fifo_data.empty:
            plt.plot(fifo_data['CacheSize'], fifo_data['HitRatio'] * 100, 
                     marker='^', linewidth=2, color=COLORS['FIFO'], label='FIFO')

        plt.title(f"Hit Ratio vs. Cache Size ({target_requests:,} Requests)", pad=15)
        plt.xlabel("Cache Size (Number of Pages)")
        plt.ylabel("Hit Ratio (%)")
        plt.ylim(-5, 105)
        plt.legend(frameon=True, facecolor='white', edgecolor='none')
        plt.tight_layout()
        
        out_path = os.path.join(plot_dir, 'hit_ratio_vs_cache_size.png')
        plt.savefig(out_path, dpi=300)
        plt.close()
        print(f"Generated: {out_path}")
    else:
        print(f"Warning: No data found for request size {target_requests} to plot hit ratio.")

    # -------------------------------------------------------------
    # Plot 2: Execution Time vs. Request Count (for a fixed cache size, e.g. 5)
    # -------------------------------------------------------------
    target_cache = 5
    sub_df = grouped[grouped['CacheSize'] == target_cache]
    
    if not sub_df.empty:
        plt.figure(figsize=(8, 5))
        
        for impl in ['Naive', 'Heap', 'Set', 'LRU', 'FIFO']:
            impl_data = sub_df[sub_df['Implementation'] == impl].sort_values('Requests')
            if not impl_data.empty:
                plt.plot(impl_data['Requests'], impl_data['RuntimeMicroseconds'], 
                         marker='o', linewidth=2, color=COLORS[impl], label=impl)
                
        plt.title(f"Execution Time vs. Request Count (Cache Size = {target_cache})", pad=15)
        plt.xlabel("Request Count (Number of Accesses)")
        plt.ylabel("Execution Time (Microseconds)")
        plt.xscale('log')
        plt.yscale('log')
        plt.legend(frameon=True, facecolor='white', edgecolor='none')
        plt.tight_layout()
        
        out_path = os.path.join(plot_dir, 'execution_time_vs_request_count.png')
        plt.savefig(out_path, dpi=300)
        plt.close()
        print(f"Generated: {out_path}")
    else:
        print(f"Warning: No data found for cache size {target_cache} to plot execution time.")

    # -------------------------------------------------------------
    # Plot 3: Execution Time Bar Chart (Cache Size = 10, Requests = 10,000)
    # -------------------------------------------------------------
    target_cache_bar = 10
    target_requests_bar = 10000
    
    sub_df = grouped[(grouped['CacheSize'] == target_cache_bar) & 
                     (grouped['Requests'] == target_requests_bar)]
    
    if not sub_df.empty:
        plt.figure(figsize=(8, 5))
        
        # Order them in a nice sequence
        ordered_impls = ['FIFO', 'LRU', 'Naive', 'Heap', 'Set']
        runtimes = []
        colors = []
        
        for impl in ordered_impls:
            row = sub_df[sub_df['Implementation'] == impl]
            if not row.empty:
                runtimes.append(row['RuntimeMicroseconds'].values[0])
                colors.append(COLORS[impl])
            else:
                runtimes.append(0)
                colors.append('#cccccc')
                
        bars = plt.bar(ordered_impls, runtimes, color=colors, edgecolor='none', width=0.6)
        
        # Add values on top of the bars
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                plt.text(bar.get_x() + bar.get_width()/2.0, height + (height * 0.02),
                         f"{height:.2f} us", ha='center', va='bottom', fontsize=9)
                
        plt.title(f"Runtime Comparison (Cache Size = {target_cache_bar}, Requests = {target_requests_bar:,})", pad=15)
        plt.xlabel("Implementation")
        plt.ylabel("Execution Time (Microseconds)")
        # Allow room for labels on top of the bars
        plt.ylim(0, max(runtimes) * 1.15)
        plt.tight_layout()
        
        out_path = os.path.join(plot_dir, 'execution_time_bar_chart.png')
        plt.savefig(out_path, dpi=300)
        plt.close()
        print(f"Generated: {out_path}")
    else:
        print(f"Warning: No data found for cache={target_cache_bar}, requests={target_requests_bar} to plot bar chart.")

if __name__ == "__main__":
    main()
