#pragma once

#include <string>
#include <vector>

struct SimulationResult
{
    int hits = 0;
    int misses = 0;
    std::vector<int> finalCache;

    [[nodiscard]] int totalRequests() const;
    [[nodiscard]] double hitRatio() const;
    [[nodiscard]] double missRatio() const;
};

class BeladyAlgorithm
{
public:
    virtual int simulate(const std::vector<int>& requests, int cacheSize) = 0;
    virtual SimulationResult simulateDetailed(const std::vector<int>& requests,
                                              int cacheSize) = 0;
    virtual std::string name() const = 0;
    virtual ~BeladyAlgorithm() = default;
};
