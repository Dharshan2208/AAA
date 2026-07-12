# Cache Replacement Algorithm Analysis

This document provides a comprehensive design choice overview, algorithmic complexity breakdown, and experimental performance analysis comparing the offline optimal (Belady's) algorithm against standard online caching heuristics (LRU and FIFO).

---

## 4. Implementation and Design Choices

### 4.1 Data Structure Used

To analyze the performance tradeoffs of optimal caching, we implemented three distinct variations of Belady's algorithm alongside two baseline online heuristics:

1. **Belady (Naive)**: 
   This implementation utilizes a simple `std::vector<int>` to represent the cache slots and a `std::unordered_set<int>` for fast $O(1)$ membership checks. When an eviction is necessary, the algorithm performs a sequential forward scan through the remaining requests to find the next occurrence of each page currently in the cache.

2. **Belady (Heap)**:
   To optimize the search for the victim page, this variant uses a max-heap (`std::priority_queue`) that stores pairs of `(next_occurrence_index, page_number)`. Future occurrences for all pages are precomputed and stored in a lookup map of queues (`std::unordered_map<int, std::queue<int>>`). Because updating elements inside a standard C++ priority queue is inefficient, we employ a lazy deletion strategy: when a page's next occurrence changes, we insert a new entry into the heap and use an auxiliary map (`std::unordered_map<int, int>`) to track and discard stale heap entries during eviction.

3. **Belady (Set)**:
   This version stores the current cache contents in a balanced binary search tree (`std::set<std::pair<int, int>>`), ordered by each page's next usage index. Unlike the heap variant, a `std::set` allows for eager updates. On a page hit, the old `(next_occurrence, page)` pair is searched, erased, and re-inserted with the updated next-use index. This ensures the data structure size never exceeds the cache capacity $C$.

4. **Least Recently Used (LRU)**:
   LRU is implemented using a doubly-linked list (`std::list<int>`) to maintain access recency (with the most recently used page at the front) and a hash map (`std::unordered_map<int, std::list<int>::iterator>`) mapping page numbers to their corresponding list node iterators. This enables $O(1)$ updates and evictions.

5. **First-In, First-Out (FIFO)**:
   FIFO utilizes a standard `std::queue<int>` to track the page insertion order and a `std::unordered_set<int>` to perform instantaneous membership lookups.

---

### 4.2 Algorithmic Complexity

The following table summarizes the time complexity per request and the auxiliary space complexity for each caching implementation, where $N$ is the total number of page requests and $C$ is the cache capacity.

| Algorithm | Best-Case Time | Worst-Case Time | Auxiliary Space Complexity |
| :--- | :--- | :--- | :--- |
| **Belady (Naive)** | $O(1)$ | $O(N \cdot C)$ | $O(C)$ |
| **Belady (Heap)** | $O(1)$ | $O(\log N)$ | $O(N + C)$ |
| **Belady (Set)** | $O(1)$ | $O(\log C)$ | $O(N + C)$ |
| **LRU** | $O(1)$ | $O(1)$ | $O(C)$ |
| **FIFO** | $O(1)$ | $O(1)$ | $O(C)$ |

- **Belady Naive**: On a cache miss when the cache is full, it performs a linear scan of length up to $N - i$ (where $i$ is the current request index) for each of the $C$ cached pages. This results in a worst-case time complexity of $O(N \cdot C)$ per request, or $O(N^2 \cdot C)$ overall.
- **Belady Heap**: Precomputing the future occurrences takes $O(N)$ time. In the worst case (where every page is unique), the heap can grow to size $N$ due to lazy deletion, making individual heap operations take $O(\log N)$ time.
- **Belady Set**: Precomputing occurrences takes $O(N)$ time. Because we eagerly delete and update elements on hits, the set size is strictly bounded by the cache capacity $C$, resulting in a guaranteed $O(\log C)$ time complexity per request.
- **LRU & FIFO**: Both algorithms perform constant-time pointer or queue updates, guaranteeing $O(1)$ time complexity per request.

---

### 4.3 Justification

- **Belady Naive**: Offers the simplest implementation with zero precomputation overhead and minimal memory footprint ($O(C)$). It is ideal for small request lists but becomes unviable as the request count grows.
- **Belady Heap**: Utilizing a priority queue is a natural choice because Belady's algorithm must evict the page with the maximum next-use index. A max-heap provides $O(1)$ access to this maximum element.
- **Belady Set**: While a heap is faster for insertion, a balanced binary search tree (`std::set`) is chosen here because it supports efficient arbitrary deletions. This allows us to keep the data structure clean of stale entries, bounding its size to $C$.
- **LRU & FIFO**: These are standard, practical online algorithms. LRU's combination of a list and hash map is the standard design to achieve true $O(1)$ operational overhead, while FIFO serves as a minimal-overhead baseline.

---

## 5. Performance Analysis and Experimental Evaluation

### 5.1 Correctness Verification

To guarantee the correctness of the three Belady implementations, the benchmark suite runs a validation check before executing the timing benchmarks. Since Belady's algorithm is deterministic, all three variants must return the exact same hit and miss counts for any given input sequence. 

Across all test runs (including standard random sequences and larger test cases like `cache5_5000` and `cache10_10000`), the correctness check successfully passed:
```
Belady Correctness Check: PASSED (Naive, Heap, Set)
```
This confirms that both the lazy heap approach and the eager set-based tracker accurately mirror the behavior of the reference naive scanner.

---

### 5.2 Parameter Testing (Cache Size)

We evaluated the influence of cache size on the overall hit ratio by running tests with cache sizes ranging from 3 to 100 on random sequences. As expected, increasing the cache size improves the hit ratio for all algorithms. However, Belady's algorithm always defines the upper bound.

Representative data points from the experimental runs:
- **Cache Size 3 (100 Requests)**:
  - Belady: **50.00%** Hit Ratio
  - LRU: **30.00%** Hit Ratio
  - FIFO: **38.00%** Hit Ratio
- **Cache Size 5 (100 Requests)**:
  - Belady: **71.00%** Hit Ratio
  - LRU: **53.00%** Hit Ratio
  - FIFO: **53.00%** Hit Ratio
- **Cache Size 10 (10,000 Requests)**:
  - Belady: **75.88%** Hit Ratio
  - LRU: **49.82%** Hit Ratio
  - FIFO: **50.10%** Hit Ratio

---

### 5.3 Implementation Comparison (Belady vs. LRU vs FIFO)

The following table presents a head-to-head comparison of hit ratios and runtime execution speeds on a large test case (`cache5_5000.txt`):

| Algorithm | Hits | Misses | Hit Ratio | Avg Execution Time (us) |
| :--- | :--- | :--- | :--- | :--- |
| **FIFO** | 2481 | 2519 | 49.62% | 116.65 us |
| **LRU** | 2446 | 2554 | 48.92% | 244.09 us |
| **Belady (Naive)** | 3619 | 1381 | 72.38% | 245.70 us |
| **Belady (Heap)** | 3619 | 1381 | 72.38% | 401.89 us |
| **Belady (Set)** | 3619 | 1381 | 72.38% | 493.68 us |

---

### 5.4 Discussion of Results

1. **Hit Ratio Superiority**: Belady's algorithm consistently achieves a significantly higher hit ratio (~72.4% compared to ~49% for LRU and FIFO on 5,000 requests). This confirms that having complete future knowledge allows for optimal eviction decisions.
2. **Online Overhead Trade-offs**: In online caching, FIFO is roughly twice as fast as LRU (116.65 us vs. 244.09 us). This is because FIFO only updates its queue on cache misses, whereas LRU must update its list on every single hit to maintain recency ordering, incurring substantial pointer manipulation overhead.
3. **Offline Lookahead Overhead**: The need to scan or maintain future lookups makes all three Belady implementations slower than the online heuristics. Interestingly, for a small cache size like $C=5$, the **Naive** linear scanner actually outperforms the Heap and Set versions in execution time (245.70 us vs. 401.89 us and 493.68 us). While asymptotically slower, the naive approach avoids the memory allocation and pointer-chasing overhead associated with C++ node-based trees (`std::set`) and vector-backed priority queues.
4. **Heap vs. Set Runtimes**: The Heap implementation is faster than the Set version. Even though the heap grows larger due to lazy deletion, it is backed by a contiguous `std::vector` which benefits from excellent cache locality. In contrast, the Set implementation uses a red-black tree, resulting in frequent dynamic allocations and poor cache line utilization.

---

## 6. Conclusion

In this analysis, we compared several caching strategies, focusing on the offline optimal Belady algorithm and its different implementation variations. 

Our experimental results demonstrate a clear trade-off between decision quality and computational efficiency. The three C++ implementations of Belady’s algorithm (Naive, Heap, and Set) consistently deliver the highest possible hit ratio, proving the power of future lookahead. However, this optimality comes at a cost: because these implementations must either perform linear scans or maintain priority queues/balanced search trees to evaluate future access times, they are significantly slower than standard online heuristics. In comparison, LRU and FIFO run much faster due to their $O(1)$ operations, but they achieve inferior hit ratios because they make decisions based solely on past access history or arrival order rather than future demands. 

Ultimately, while Belady's algorithm serves as an invaluable theoretical benchmark for optimal cache performance, its high computational overhead and requirement of offline knowledge make it less suited for real-time systems where simple, fast heuristics like LRU remain the practical choice.
