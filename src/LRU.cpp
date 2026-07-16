#include "LRU.hpp"

#include "Utils.hpp"

#include <list>
#include <unordered_map>

int LRU::simulate(const std::vector<int>& requests, int cacheSize)
{
    return simulateDetailed(requests, cacheSize).misses;
}

SimulationResult LRU::simulateDetailed(const std::vector<int>& requests,
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

    std::list<int> recencyOrder; // Front is most recently used.
    std::unordered_map<int, std::list<int>::iterator> cachePositions;

    for (const int page : requests) {
        const auto found = cachePositions.find(page);
        if (found != cachePositions.end()) {
            ++result.hits;
            recencyOrder.erase(found->second);
            recencyOrder.push_front(page);
            cachePositions[page] = recencyOrder.begin();
            continue;
        }

        ++result.misses;
        if (static_cast<int>(cachePositions.size()) == cacheSize) {
            const int victim = recencyOrder.back();
            recencyOrder.pop_back();
            cachePositions.erase(victim);
        }

        recencyOrder.push_front(page);
        cachePositions[page] = recencyOrder.begin();
    }

    return result;
}

std::string LRU::name() const
{
    return "LRU";
}
