#pragma once

#include "BeladyAlgorithm.hpp"

class BeladyHeap final : public BeladyAlgorithm
{
public:
    int simulate(const std::vector<int>& requests, int cacheSize) override;
    SimulationResult simulateDetailed(const std::vector<int>& requests,
                                      int cacheSize) override;
    std::string name() const override;
};
