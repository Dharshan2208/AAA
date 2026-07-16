#include "FIFO.hpp"

#include "Utils.hpp"

#include <queue>
#include <unordered_set>

int FIFO::simulate(const std::vector<int>& requests, int cacheSize)
{
    return simulateDetailed(requests, cacheSize).misses;
}

SimulationResult FIFO::simulateDetailed(const std::vector<int>& requests,
                                        int cacheSize)
{
    SimulationResult result;
    if (requests.empty()) {
        return result;
    }
    if (cacheSize <= 0) {
        result.misses = static_cast<int>(requests.size());
        return result;
    }

    std::queue<int> insertionOrder;
    std::unordered_set<int> cache;

    for (const int page : requests) {
        if (cache.contains(page)) {
            ++result.hits;
            continue;
        }

        ++result.misses;
        if (static_cast<int>(cache.size()) == cacheSize) {
            const int victim = insertionOrder.front();
            insertionOrder.pop();
            cache.erase(victim);
        }

        cache.insert(page);
        insertionOrder.push(page);
    }

    return result;
}

std::string FIFO::name() const
{
    return "FIFO";
}
