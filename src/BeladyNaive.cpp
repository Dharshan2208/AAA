#include "BeladyNaive.hpp"

#include "Utils.hpp"

#include <algorithm>
#include <unordered_set>

namespace
{
int nextOccurrenceByScan(const std::vector<int>& requests,
                         int startIndex,
                         int page)
{
    for (int i = startIndex; i < static_cast<int>(requests.size()); ++i) {
        if (requests[static_cast<std::size_t>(i)] == page) {
            return i;
        }
    }
    return Utils::infinityIndex;
}
}

int BeladyNaive::simulate(const std::vector<int>& requests, int cacheSize)
{
    return simulateDetailed(requests, cacheSize).misses;
}

SimulationResult BeladyNaive::simulateDetailed(const std::vector<int>& requests,
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

    std::vector<int> cache;
    std::unordered_set<int> inCache;
    cache.reserve(static_cast<std::size_t>(cacheSize));

    for (int i = 0; i < static_cast<int>(requests.size()); ++i) {
        const int page = requests[static_cast<std::size_t>(i)];
        if (inCache.contains(page)) {
            ++result.hits;
            continue;
        }

        ++result.misses;
        if (static_cast<int>(cache.size()) < cacheSize) {
            cache.push_back(page);
            inCache.insert(page);
            continue;
        }

        int victimIndex = 0;
        int farthestNextUse = -1;
        for (int j = 0; j < static_cast<int>(cache.size()); ++j) {
            const int nextUse = nextOccurrenceByScan(requests, i + 1, cache[static_cast<std::size_t>(j)]);
            const int currentVictim = cache[static_cast<std::size_t>(victimIndex)];
            const int candidate = cache[static_cast<std::size_t>(j)];
            if (nextUse > farthestNextUse ||
                (nextUse == farthestNextUse && candidate > currentVictim)) {
                farthestNextUse = nextUse;
                victimIndex = j;
            }
        }

        inCache.erase(cache[static_cast<std::size_t>(victimIndex)]);
        cache[static_cast<std::size_t>(victimIndex)] = page;
        inCache.insert(page);
    }

    return result;
}

std::string BeladyNaive::name() const
{
    return "Naive";
}
