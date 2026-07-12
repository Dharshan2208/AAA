#include "BeladyHeap.hpp"
#include "BeladyNaive.hpp"
#include "BeladySet.hpp"
#include "Benchmark.hpp"
#include "FIFO.hpp"
#include "LRU.hpp"
#include "Utils.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace
{
std::vector<std::unique_ptr<BeladyAlgorithm>> createOptimalAlgorithms()
{
    std::vector<std::unique_ptr<BeladyAlgorithm>> algorithms;
    algorithms.push_back(std::make_unique<BeladyNaive>());
    algorithms.push_back(std::make_unique<BeladyHeap>());
    algorithms.push_back(std::make_unique<BeladySet>());
    return algorithms;
}

std::vector<std::unique_ptr<BeladyAlgorithm>> createBenchmarkAlgorithms()
{
    auto algorithms = createOptimalAlgorithms();
    algorithms.push_back(std::make_unique<LRU>());
    algorithms.push_back(std::make_unique<FIFO>());
    return algorithms;
}

void printProgramHeader(int testcaseCount, int benchmarkIterations)
{
    constexpr int width = 78;
    std::cout << std::string(width, '=') << '\n'
              << "Page Replacement Benchmark Suite\n"
              << std::string(width, '=') << '\n'
              << "Testcases loaded      : " << testcaseCount << '\n'
              << "Max benchmark runs    : " << benchmarkIterations << '\n'
              << std::string(width, '=') << "\n\n";
}

void printTestcaseHeader(const TestCase& testCase)
{
    constexpr int width = 78;
    std::cout << std::string(width, '-') << '\n'
              << "Testcase: " << testCase.name << '\n'
              << "Cache size: " << testCase.cacheSize
              << " | Requests: " << testCase.requests.size() << '\n'
              << std::string(width, '-') << '\n';
}

int iterationsForRequestCount(std::size_t requestCount)
{
    if (requestCount <= 1'000) {
        return 1'000;
    }
    if (requestCount <= 10'000) {
        return 100;
    }
    if (requestCount <= 50'000) {
        return 20;
    }
    return 5;
}

void printCorrectnessTableHeader()
{
    std::cout << "\nBelady Correctness Check: PASSED (Naive, Heap, Set)\n"
              << "Algorithm Results\n"
              << std::string(73, '-') << '\n'
              << std::left << std::setw(16) << "Implementation"
              << std::right << std::setw(8) << "Cache"
              << std::setw(10) << "Requests"
              << std::setw(8) << "Hits"
              << std::setw(9) << "Misses"
              << std::setw(11) << "Hit Ratio"
              << std::setw(11) << "Miss Ratio" << '\n'
              << std::string(73, '-') << '\n';
}

int parsePositiveInteger(const char* value, const std::string& name)
{
    const int parsed = std::stoi(value);
    if (parsed <= 0) {
        throw std::invalid_argument(name + " must be positive");
    }
    return parsed;
}

std::vector<TestCase> filterTestCases(const std::vector<TestCase>& testCases,
                                      std::optional<int> cacheSize,
                                      std::optional<int> requestCount)
{
    if (!cacheSize && !requestCount) {
        return testCases;
    }

    std::vector<TestCase> filtered;
    for (const auto& testCase : testCases) {
        if (testCase.cacheSize == *cacheSize &&
            static_cast<int>(testCase.requests.size()) == *requestCount) {
            filtered.push_back(testCase);
        }
    }
    return filtered;
}
}

int main(int argc, char* argv[])
{
    try {
        if (argc != 1 && argc != 3) {
            std::cerr << "Usage: ./build/optimal_caching [cache_size request_count]\n";
            return 1;
        }

        std::optional<int> requestedCacheSize;
        std::optional<int> requestedRequestCount;
        if (argc == 3) {
            requestedCacheSize = parsePositiveInteger(argv[1], "cache_size");
            requestedRequestCount = parsePositiveInteger(argv[2], "request_count");
        }

        const std::filesystem::path testcaseDirectory = "testcases";
        const std::filesystem::path resultsDirectory = "results";
        const std::filesystem::path csvPath = resultsDirectory / "benchmark_results.csv";
        constexpr int benchmarkIterations = 1000;

        Utils::ensureDefaultTestCases(testcaseDirectory);
        Utils::ensureDirectory(resultsDirectory);
        std::ofstream(csvPath, std::ios::trunc).close();

        const auto allTestCases = Utils::loadTestCases(testcaseDirectory);
        const auto testCases = filterTestCases(allTestCases,
                                               requestedCacheSize,
                                               requestedRequestCount);
        if (testCases.empty()) {
            std::cerr << "No matching testcases found";
            if (requestedCacheSize && requestedRequestCount) {
                std::cerr << " for cache size " << *requestedCacheSize
                          << " and request count " << *requestedRequestCount;
            }
            std::cerr << ".\n";
            return 1;
        }

        printProgramHeader(static_cast<int>(testCases.size()),
                           benchmarkIterations);

        for (const auto& testCase : testCases) {
            auto optimalAlgorithms = createOptimalAlgorithms();
            auto benchmarkAlgorithms = createBenchmarkAlgorithms();
            const int iterations =
                iterationsForRequestCount(testCase.requests.size());
            const Benchmark benchmark(iterations);
            printTestcaseHeader(testCase);
            std::cout << "Benchmark runs: " << iterations << '\n';

            if (!benchmark.verifyImplementations(optimalAlgorithms,
                                                 testCase.requests,
                                                 testCase.cacheSize)) {
                return 1;
            }

            printCorrectnessTableHeader();
            for (const auto& algorithm : benchmarkAlgorithms) {
                const SimulationResult result =
                    algorithm->simulateDetailed(testCase.requests,
                                                testCase.cacheSize);
                Utils::printImplementationReport(*algorithm,
                                                 result,
                                                 testCase.cacheSize);
            }
            std::cout << std::string(73, '-') << '\n';

            const auto records = benchmark.run(benchmarkAlgorithms,
                                               testCase.requests,
                                               testCase.cacheSize);
            benchmark.printComparisonTable(records);
            benchmark.printRanking(records);
            benchmark.appendCsv(csvPath, records);
            std::cout << '\n';
        }

        std::cout << "CSV output: " << csvPath << '\n';
    } catch (const std::exception& exception) {
        std::cerr << "[FATAL] " << exception.what() << '\n';
        return 1;
    }

    return 0;
}
