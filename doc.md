# Cache Replacement Algorithm Analysis

This document provides a comprehensive design choice overview, algorithmic complexity breakdown, and experimental performance analysis comparing the offline optimal (Belady's) algorithm against standard online caching heuristics (LRU and FIFO).

---

## 3. Formal Proof of Correctness

### Theorem
For any request sequence $\sigma = \langle r_1, r_2, \dots, r_n \rangle$ and a cache capacity $K$, Belady’s Farthest-in-Future (FIF) algorithm produces an eviction schedule with the minimum number of cache misses among all cache replacement algorithms.

### 3.1 Initial Assumptions and Inductive Step
Let $\sigma = \langle r_1, r_2, \dots, r_n \rangle$ be a request sequence over a finite page universe, and let $K$ be the cache capacity. A schedule is considered *reduced* if it brings a page into the cache only when the page is requested and is not already present. Thus, every cache miss corresponds to exactly one page insertion.

Consider any arbitrary optimal schedule $S$ and Belady’s Farthest-in-Future schedule $S_{FF}$. Both schedules begin with the same initial cache contents $C_0$.

We define that two schedules agree on the first $j$ requests if they produce the same cache contents after each of $r_1, \dots, r_j$. Concretely, this means they make identical eviction choices at every miss within those $j$ requests, resulting in identical cache contents after processing each request.

*   **Base Case ($j = 0$):** Before any request is processed, both $S$ and $S_{FF}$ start with the same initial cache contents $C_0$. No requests have been processed, no evictions have occurred, and both schedules incur zero cache misses. Therefore, they agree trivially on the first 0 requests:
    $$\text{misses}(S, \sigma[1..0]) = \text{misses}(S_{FF}, \sigma[1..0]) = 0$$

*   **Inductive Assumption:** Assume that for some $j \ge 0$, there exists a reduced schedule $S$ (which is optimal) that agrees with $S_{FF}$ on the first $j$ requests. Because they are reduced and started from the same $C_0$, they hold identical cache contents $C_j$ at the start of request $r_{j+1}$.

*   **Inductive Goal:** Construct another reduced schedule $S'$ that agrees with $S_{FF}$ on the first $j + 1$ requests while incurring no more cache misses than $S$:
    $$\text{misses}(S', \sigma[1..n]) \le \text{misses}(S, \sigma[1..n])$$
    This exchange step will allow us to prove the optimality of Belady's algorithm.

---

### 3.2 The Exchange Argument
We consider what happens at step $j+1$ to construct $S'$. Exactly one of three cases occurs:

*   **Case 1: Cache Hit**
    Since $S$ and $S_{FF}$ share the same cache state $C_j$, a hit for one is a hit for the other. No eviction occurs. Both load nothing and change nothing. We set $S' = S$. The schedules still agree on $j+1$ requests, and the miss count does not change:
    $$\text{misses}(S', \sigma) = \text{misses}(S, \sigma)$$

*   **Case 2: Cache Miss (Cache Not Full)**
    Both schedules miss on $r_{j+1}$. Since $|C_j| < K$, $r_{j+1}$ is simply loaded with no eviction needed. Both make the exact same decision. We set $S' = S$. Agreement on $j+1$ requests holds, and the miss count remains unchanged.

*   **Case 3: Cache Miss (Cache Full) - The Critical Case**
    Both schedules miss on $r_{j+1}$ and $|C_j| = K$, so one page must be evicted by each. Let $e_{FF}$ be the page $S_{FF}$ evicts (the one whose next use is farthest in the future). Let $e_S$ be the page $S$ evicts (arbitrary, under its own rule).

    If $e_S = e_{FF}$, both make the same eviction. We set $S' = S$, and the agreement extends to $j+1$ requests trivially. 
    
    If $e_S \neq e_{FF}$, an exchange must happen. Because $S_{FF}$ chose $e_{FF}$ as the farthest-future page, we know:
    $$\tau_{j+1}(e_{FF}) \ge \tau_{j+1}(e_S)$$
    where $\tau_{j+1}(p)$ denotes the index of the next request for page $p$.

    **Constructing $S'$:** We define $S'$ to make identical decisions to $S$ on all requests, except at step $j+1$ it evicts $e_{FF}$ instead of $e_S$. After step $j+1$, $S'$ and $S_{FF}$ now hold identical cache contents: $(C_j \setminus \{e_{FF}\}) \cup \{r_{j+1}\}$. Thus, $S'$ successfully agrees with $S_{FF}$ on the first $j+1$ requests.

    **Comparing Misses between $S$ and $S'$:** After the exchange at step $j+1$, $S$ has $e_{FF}$ in its cache and $e_S$ is out; $S'$ has $e_S$ in its cache and $e_{FF}$ is out. Let $t_S = \tau_{j+1}(e_S)$ be the next request for $e_S$, and $t_{FF} = \tau_{j+1}(e_{FF})$ be the next request for $e_{FF}$, where $t_{FF} \ge t_S$.

    Between steps $j+2$ and $t_S - 1$, neither $e_S$ nor $e_{FF}$ is requested. Thus, $S$ and $S'$ handle every request identically and incur the same misses. At step $t_S$, $e_S$ is requested. $S$ incurs a miss because it evicted $e_S$ at step $j+1$, meaning it must bring $e_S$ in and evict some page $x$. Meanwhile, $S'$ hits on $e_S$ because it kept it. This is where $S'$ gains an advantage.

    We now evaluate based on $t_{FF}$ and the evicted page $x$:
    *   **Sub-case A: $t_{FF} = \infty$ (eFF is never requested again).**
        $e_{FF}$ is in $S$'s cache but will never be needed. It was evicted from $S'$ at step $j+1$ with no future consequence. $S$ takes 1 miss at $t_S$, while $S'$ takes 0. Thus:
        $$\text{misses}(S') < \text{misses}(S)$$
    *   **Sub-case B: $t_{FF} < \infty$ and $x \neq e_{FF}$ ($S$ keeps $e_{FF}$ when evicting at $t_S$).**
        At $t_S$, $S$ evicts $x \neq e_{FF}$, keeping $e_{FF}$ in its cache. At step $t_{FF}$, $e_{FF}$ is requested, and $S$ hits on it. $S'$ misses on $e_{FF}$, brings it in, and evicts $x$. Over the interval $[t_S, t_{FF}]$, both schedules have incurred exactly 1 miss ($S$ at $t_S$ and $S'$ at $t_{FF}$). After $t_{FF}$, their caches are synchronized (both have $e_S$ and $e_{FF}$, and both are missing $x$). Thus:
        $$\text{misses}(S') \le \text{misses}(S)$$
    *   **Sub-case C: $t_{FF} < \infty$ and $x = e_{FF}$ ($S$ evicts $e_{FF}$ when handling $t_S$).**
        At $t_S$, $S$ evicts $e_{FF}$ to bring $e_S$ in. Now neither schedule has $e_{FF}$. At step $t_{FF}$, both schedules will miss on $e_{FF}$. Over the interval, $S$ incurs 2 misses ($t_S$ and $t_{FF}$), while $S'$ incurs only 1 miss ($t_{FF}$). Thus:
        $$\text{misses}(S') < \text{misses}(S)$$

---

### 3.3 Conclusion of Proof
In every possible scenario, the constructed schedule $S'$ incurs at most the number of misses as the optimal schedule $S$:
$$\text{misses}(S') \le \text{misses}(S)$$
By induction, we can transform any optimal schedule $S$ into the Farthest-in-Future schedule $S_{FF}$ step-by-step without ever increasing the total number of cache misses. Therefore, Belady's Farthest-in-Future algorithm is mathematically optimal.

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

To validate the programmatic correctness of the optimal caching implementations, the benchmark suite executes an automated verification routine prior to running the performance benchmarks. 

According to the **Theorem of Optimality** (detailed in Section 3), Belady’s Farthest-in-Future algorithm yields a mathematically unique minimum number of cache misses for any request sequence $\sigma$ and cache capacity $K$. Since the Naive scanner, binary Heap, and balanced Set implementations are all distinct programmatic realizations of this same Farthest-in-Future eviction policy, they must generate identical hit and miss counts for any given input sequence to be correct.

Before executing the timing trials, the suite simulates the request sequence across all three Belady variants and confirms that their outputs match exactly. The following table showcases the correctness results obtained from the `cache5_1000` test case (cache size 5, 1,000 requests):

| Implementation | Cache Size | Requests | Hits | Misses | Hit Ratio (%) | Miss Ratio (%) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Naive** | 5 | 1000 | 64 | 936 | 6.40% | 93.60% |
| **Heap** | 5 | 1000 | 64 | 936 | 6.40% | 93.60% |
| **Set** | 5 | 1000 | 64 | 936 | 6.40% | 93.60% |

Across all experimental test runs (including standard random sequences and larger test cases like `cache5_5000` and `cache10_10000`), the correctness check consistently reports:
```
Belady Correctness Check: PASSED (Naive, Heap, Set)
```
This empirical verification confirms that the lazy deletion updates in the heap and the eager index adjustments in the set do not introduce logic errors, showing that all three optimized implementations behave identically to the theoretically proven optimal policy.

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

![Hit Ratio vs Cache Size](/home/ani/aaa/results/plots/hit_ratio_vs_cache_size.png)

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

![Execution Time vs Request Count](/home/ani/aaa/results/plots/execution_time_vs_request_count.png)

![Combined Performance (Hit Ratio & Execution Time)](/home/ani/aaa/results/plots/combined_performance.png)

![Algorithm Performance Grid (Cache 50 & 100)](/home/ani/aaa/results/plots/cache_50_100_comparison.png)

---

### 5.4 Discussion of Results

1. **Hit Ratio Superiority**: Belady's algorithm consistently achieves a significantly higher hit ratio (~72.4% compared to ~49% for LRU and FIFO on 5,000 requests). This confirms that having complete future knowledge allows for optimal eviction decisions.
2. **Online Overhead Trade-offs**: In online caching, FIFO is roughly twice as fast as LRU (116.65 us vs. 244.09 us). This is because FIFO only updates its queue on cache misses, whereas LRU must update its list on every single hit to maintain recency ordering, incurring substantial pointer manipulation overhead.
3. **Offline Lookahead Overhead**: The need to scan or maintain future lookups makes all three Belady implementations slower than the online heuristics. Interestingly, for a small cache size like $C=5$, the **Naive** linear scanner actually outperforms the Heap and Set versions in execution time (245.70 us vs. 401.89 us and 493.68 us). While asymptotically slower, the naive approach avoids the memory allocation and pointer-chasing overhead associated with C++ node-based trees (`std::set`) and vector-backed priority queues.

---

## 6. Conclusion

In this analysis, we compared several caching strategies, focusing on the offline optimal Belady algorithm and its different implementation variations. 

Our experimental results demonstrate a clear trade-off between decision quality and computational efficiency. The three C++ implementations of Belady’s algorithm (Naive, Heap, and Set) consistently deliver the highest possible hit ratio, proving the power of future lookahead. However, this optimality comes at a cost: because these implementations must either perform linear scans or maintain priority queues/balanced search trees to evaluate future access times, they are significantly slower than standard online heuristics. In comparison, LRU and FIFO run much faster due to their $O(1)$ operations, but they achieve inferior hit ratios because they make decisions based solely on past access history or arrival order rather than future demands. 

Ultimately, while Belady's algorithm serves as an invaluable theoretical benchmark for optimal cache performance, its high computational overhead and requirement of offline knowledge make it less suited for real-time systems where simple, fast heuristics like LRU remain the practical choice.
