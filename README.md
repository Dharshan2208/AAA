# Page Replacement Caching Benchmark Suite

A C++23 benchmark framework designed to evaluate and compare offline optimal caching algorithms (Belady's Farthest-in-Future) and standard online heuristics (LRU, FIFO). The repository features multiple implementations of Belady's algorithm to analyze data structure overheads and includes a comprehensive evaluation and plotting pipeline.

---

## Repository Structure

```text
├── include/                   # Header files for caching algorithms
│   ├── BeladyAlgorithm.hpp    # Abstract base class for Belady variants
│   ├── BeladyNaive.hpp        # Naive linear lookahead variant
│   ├── BeladyHeap.hpp         # Lazy heap-based lookahead variant
│   ├── BeladySet.hpp          # Eager set-based lookahead variant
│   ├── FIFO.hpp               # FIFO replacement policy
│   ├── LRU.hpp                # LRU replacement policy
│   ├── Benchmark.hpp          # Performance benchmarking harness
│   └── Utils.hpp              # File IO and request generation helpers
├── src/                       # Implementation source files
│   ├── BeladyNaive.cpp
│   ├── BeladyHeap.cpp
│   ├── BeladySet.cpp
│   ├── FIFO.cpp
│   ├── LRU.cpp
│   ├── Benchmark.cpp
│   ├── Utils.cpp
│   └── main.cpp               # CLI entry point
├── testcases/                 # Generated request sequence files
├── results/                   # Benchmark CSV logs and output plots
│   └── plots/                 # Visual performance charts
├── CMakeLists.txt             # CMake configuration
├── Makefile                   # Quick commands for build and run
├── doc.md                     # Academic report with proof & analysis
├── generate_testcases.py      # Automated request generator
├── generate_plots.py          # Primary plotting script
├── generate_combined_plot.py # Dual-axis performance chart script
└── generate_grid_plot.py     # Side-by-side cache size comparison script
```

---

## Implemented Algorithms

| Algorithm | time complexity (per request) | space complexity | updates |
| :--- | :---: | :---: | :--- |
| **Belady (Naive)** | $O(N \cdot C)$ | $O(C)$ | Linear scan forward to find the next occurrence of cached pages on miss. |
| **Belady (Heap)** | $O(\log N)$ | $O(N + C)$ | Precomputes occurrences; uses a max-heap with lazy deletion of stale entries. |
| **Belady (Set)** | $O(\log C)$ | $O(N + C)$ | Precomputes occurrences; uses a balanced BST (`std::set`) with eager updates. |
| **LRU** | $O(1)$ | $O(C)$ | Standard doubly-linked list with hash map lookup for true constant time. |
| **FIFO** | $O(1)$ | $O(C)$ | Queue + hash set for immediate lookup and entry. |

---

## Getting Started

### Prerequisites

- A C++23 compliant compiler (e.g., GCC 12+, Clang 15+)
- CMake (version 3.15+)
- Make
- Python 3 with `pandas` and `matplotlib` (for generating graphs)

### Compilation

Build the project using `make`:
```bash
make
```
This builds the binary in `./build/optimal_caching`.

### Running the Benchmarks

#### Run default test suite:
```bash
./build/optimal_caching
```

#### Run with custom parameters:
You can run the benchmark using the Makefile with dynamic inputs (commas in numbers are handled automatically):
```bash
make run 5 5000
make run 10 10,000
```
This automatically generates the appropriate request file in `/testcases/` if it does not already exist, verifies correctness across all Belady variants, runs the timing suite, and appends the records to `results/benchmark_results.csv`.

---

## Plotting & Visualization

To generate the performance plots:

1. **Activate Python Virtual Environment**:
   ```bash
   source venv/bin/activate
   ```
2. **Generate Charts**:
   - **General benchmark plots** (hit ratios vs. cache size, log runtime scaling):
     ```bash
     python3 generate_plots.py
     ```
   - **Combined dual-axis chart** (Hit ratio and execution time combined on a single plot for a given config):
     ```bash
     python3 generate_combined_plot.py
     ```
   - **Side-by-side comparative grid** (Compares hit ratio and runtime across request sizes for cache sizes 50 and 100):
     ```bash
     python3 generate_grid_plot.py
     ```

The output charts are saved as PNG files inside `results/plots/`.

---

## Correctness & Analysis Report

For a complete academic analysis of the system, check [doc.md](doc.md). It contains:
- A formal mathematical proof of the optimality of Belady's algorithm (inductive step and exchange arguments).
- Concrete data structure choice explanations.
- A comprehensive experimental evaluation of hit ratios and runtime profiles.
