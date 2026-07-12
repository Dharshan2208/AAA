#include "BeladyHeap.hpp"

#include "Utils.hpp"

#include <queue>
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

int BeladyHeap::simulate(const std::vector<int>& requests, int cacheSize)
{
    return simulateDetailed(requests, cacheSize).misses;
}

SimulationResult BeladyHeap::simulateDetailed(const std::vector<int>& requests,
                                             int cacheSize)
{
    SimulationResult result;
    if (cacheSize <= 0) {
        result.misses = static_cast<int>(requests.size());
        return result;
    }

    using HeapEntry = std::pair<int, int>; // (next occurrence, page)
    std::priority_queue<HeapEntry> heap;
    std::unordered_map<int, int> currentNextOccurrence;
    std::unordered_set<int> cache;
    auto futureOccurrences = buildFutureOccurrences(requests);

    for (int i = 0; i < static_cast<int>(requests.size()); ++i) {
        const int page = requests[static_cast<std::size_t>(i)];
        const bool hit = cache.contains(page);
        if (hit) {
            ++result.hits;
        } else {
            ++result.misses;
            if (static_cast<int>(cache.size()) == cacheSize) {
                while (!heap.empty()) {
                    const auto [nextUse, candidatePage] = heap.top();
                    const auto current = currentNextOccurrence.find(candidatePage);
                    if (cache.contains(candidatePage) &&
                        current != currentNextOccurrence.end() &&
                        current->second == nextUse) {
                        heap.pop();
                        cache.erase(candidatePage);
                        currentNextOccurrence.erase(candidatePage);
                        break;
                    }
                    heap.pop();
                }
            }
            cache.insert(page);
        }

        const int nextUse = consumeCurrentAndFindNext(futureOccurrences, page, i);
        currentNextOccurrence[page] = nextUse;
        heap.emplace(nextUse, page);
    }

    result.finalCache = Utils::sortedCacheFromSet(cache);
    return result;
}

std::string BeladyHeap::name() const
{
    return "Heap";
}
