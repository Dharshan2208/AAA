#include "BeladySet.hpp"

#include "Utils.hpp"

#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace
{
using OccurrenceQueue = std::queue<int>;

std::unordered_map<int, OccurrenceQueue> buildFutureOccurrences(
    const std::vector<int>& requests)
{
    std::unordered_map<int, OccurrenceQueue> occurrences;
    for (int i = 0; i < static_cast<int>(requests.size()); ++i) {
        occurrences[requests[static_cast<std::size_t>(i)]].push(i);
    }
    return occurrences;
}

int consumeCurrentAndFindNext(std::unordered_map<int, OccurrenceQueue>& occurrences,
                              int page,
                              int currentIndex)
{
    auto& positions = occurrences[page];
    while (!positions.empty() && positions.front() <= currentIndex) {
        positions.pop();
    }
    return positions.empty() ? Utils::infinityIndex : positions.front();
}
}

int BeladySet::simulate(const std::vector<int>& requests, int cacheSize)
{
    return simulateDetailed(requests, cacheSize).misses;
}

SimulationResult BeladySet::simulateDetailed(const std::vector<int>& requests,
                                            int cacheSize)
{
    SimulationResult result;
    if (cacheSize <= 0) {
        result.misses = static_cast<int>(requests.size());
        return result;
    }

    std::set<std::pair<int, int>> orderedByNextUse; // (next occurrence, page)
    std::unordered_map<int, int> currentNextOccurrence;
    std::unordered_set<int> cache;
    auto futureOccurrences = buildFutureOccurrences(requests);

    for (int i = 0; i < static_cast<int>(requests.size()); ++i) {
        const int page = requests[static_cast<std::size_t>(i)];
        const bool hit = cache.contains(page);

        if (hit) {
            ++result.hits;
            orderedByNextUse.erase({currentNextOccurrence.at(page), page});
        } else {
            ++result.misses;
            if (static_cast<int>(cache.size()) == cacheSize) {
                const auto victim = std::prev(orderedByNextUse.end());
                const int victimPage = victim->second;
                orderedByNextUse.erase(victim);
                currentNextOccurrence.erase(victimPage);
                cache.erase(victimPage);
            }
            cache.insert(page);
        }

        const int nextUse = consumeCurrentAndFindNext(futureOccurrences, page, i);
        currentNextOccurrence[page] = nextUse;
        orderedByNextUse.insert({nextUse, page});
    }

    result.finalCache = Utils::sortedCacheFromSet(cache);
    return result;
}

std::string BeladySet::name() const
{
    return "Set";
}
