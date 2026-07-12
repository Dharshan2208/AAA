#include "Benchmark.hpp"

#include "Utils.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

Benchmark::Benchmark(int iterations)
    : iterations_(iterations)
{
    if (iterations_ <= 0) {
        throw std::invalid_argument("Benchmark iterations must be positive");
    }
}

bool Benchmark::verifyImplementations(
    const std::vector<std::unique_ptr<BeladyAlgorithm>>& algorithms,
    const std::vector<int>& requests,
    int cacheSize) const
{
    if (algorithms.empty()) {
        return true;
    }

    const SimulationResult expected =
        algorithms.front()->simulateDetailed(requests, cacheSize);

    for (std::size_t i = 1; i < algorithms.size(); ++i) {
        const SimulationResult actual =
            algorithms[i]->simulateDetailed(requests, cacheSize);
        if (!Utils::sameSimulationResult(expected, actual)) {
            std::cout << "\n[ERROR] Implementation mismatch detected.\n";
            return false;
        }
    }

    return true;
}

std::vector<BenchmarkRecord> Benchmark::run(
    const std::vector<std::unique_ptr<BeladyAlgorithm>>& algorithms,
    const std::vector<int>& requests,
    int cacheSize) const
{
    std::vector<BenchmarkRecord> records;
    records.reserve(algorithms.size());

    for (const auto& algorithm : algorithms) {
        SimulationResult result;
        const auto start = std::chrono::steady_clock::now();
        for (int iteration = 0; iteration < iterations_; ++iteration) {
            result = algorithm->simulateDetailed(requests, cacheSize);
        }
        const auto end = std::chrono::steady_clock::now();

        const auto totalMicroseconds =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                .count();

        records.push_back({
            algorithm->name(),
            cacheSize,
            static_cast<int>(requests.size()),
            result.hits,
            result.misses,
            result.hitRatio(),
            result.missRatio(),
            static_cast<double>(totalMicroseconds) / iterations_
        });
    }

    return records;
}

void Benchmark::printComparisonTable(
    const std::vector<BenchmarkRecord>& records) const
{
    constexpr int tableWidth = 72;
    std::cout << "\nBenchmark Results\n"
              << std::string(tableWidth, '-') << '\n'
              << std::left << std::setw(18) << "Implementation"
              << std::right << std::setw(14) << "Avg Time(us)"
              << std::setw(8) << "Hits"
              << std::setw(10) << "Misses"
              << std::setw(12) << "Hit Ratio"
              << std::setw(12) << "Miss Ratio" << '\n'
              << std::string(tableWidth, '-') << '\n';

    for (const auto& record : records) {
        std::cout << std::left << std::setw(18) << record.implementation
                  << std::right << std::setw(14) << std::fixed
                  << std::setprecision(2) << record.runtimeMicroseconds
                  << std::setw(8) << record.hits
                  << std::setw(10) << record.misses
                  << std::setw(12) << std::setprecision(4)
                  << record.hitRatio
                  << std::setw(12) << record.missRatio << '\n';
    }
    std::cout << std::string(tableWidth, '-') << '\n';
}

void Benchmark::printRanking(const std::vector<BenchmarkRecord>& records) const
{
    std::vector<BenchmarkRecord> ranking = records;
    std::ranges::sort(ranking, [](const auto& lhs, const auto& rhs) {
        return lhs.runtimeMicroseconds < rhs.runtimeMicroseconds;
    });

    std::cout << "\nRuntime Ranking\n";
    for (std::size_t i = 0; i < ranking.size(); ++i) {
        std::cout << "  " << i + 1 << ". "
                  << std::left << std::setw(10) << ranking[i].implementation
                  << std::right << std::fixed << std::setprecision(2)
                  << ranking[i].runtimeMicroseconds << " us\n";
    }
}

void Benchmark::appendCsv(const std::filesystem::path& csvPath,
                          const std::vector<BenchmarkRecord>& records) const
{
    const bool needsHeader =
        !std::filesystem::exists(csvPath) ||
        std::filesystem::file_size(csvPath) == 0;
    std::ofstream output(csvPath, std::ios::app);
    if (!output) {
        throw std::runtime_error("Unable to write CSV: " + csvPath.string());
    }

    if (needsHeader) {
        output << "Implementation,CacheSize,Requests,Hits,Misses,RuntimeMicroseconds\n";
    }

    for (const auto& record : records) {
        output << record.implementation << ','
               << record.cacheSize << ','
               << record.requestCount << ','
               << record.hits << ','
               << record.misses << ','
               << std::fixed << std::setprecision(2)
               << record.runtimeMicroseconds << '\n';
    }
}
