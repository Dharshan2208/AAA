#pragma once

#include "BeladyAlgorithm.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

struct BenchmarkRecord
{
    std::string implementation;
    int cacheSize = 0;
    int requestCount = 0;
    int hits = 0;
    int misses = 0;
    double hitRatio = 0.0;
    double missRatio = 0.0;
    double runtimeMicroseconds = 0.0;
};

class Benchmark
{
public:
    explicit Benchmark(int iterations);

    [[nodiscard]] bool verifyImplementations(
        const std::vector<std::unique_ptr<BeladyAlgorithm>>& algorithms,
        const std::vector<int>& requests,
        int cacheSize) const;

    [[nodiscard]] std::vector<BenchmarkRecord> run(
        const std::vector<std::unique_ptr<BeladyAlgorithm>>& algorithms,
        const std::vector<int>& requests,
        int cacheSize) const;

    void printComparisonTable(const std::vector<BenchmarkRecord>& records) const;
    void printRanking(const std::vector<BenchmarkRecord>& records) const;
    void appendCsv(const std::filesystem::path& csvPath,
                   const std::vector<BenchmarkRecord>& records) const;

private:
    int iterations_;
};
