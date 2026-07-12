#include "BeladyHeap.hpp"
#include "BeladyNaive.hpp"
#include "BeladySet.hpp"
#include "Benchmark.hpp"
#include "Utils.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace
{
std::vector<std::unique_ptr<BeladyAlgorithm>> createAlgorithms()
{
    std::vector<std::unique_ptr<BeladyAlgorithm>> algorithms;
    algorithms.push_back(std::make_unique<BeladyNaive>());
    algorithms.push_back(std::make_unique<BeladyHeap>());
    algorithms.push_back(std::make_unique<BeladySet>());
    return algorithms;
}

void printProgramHeader(int testcaseCount, int benchmarkIterations)
{
    constexpr int width = 78;
    std::cout << std::string(width, '=') << '\n'
              << "Belady's Optimal Caching - Benchmark Suite\n"
              << std::string(width, '=') << '\n'
              << "Testcases loaded      : " << testcaseCount << '\n'
              << "Benchmark iterations  : " << benchmarkIterations << '\n'
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

void printCorrectnessTableHeader()
{
    std::cout << "\nCorrectness Check: PASSED\n"
              << std::string(78, '-') << '\n'
              << std::left << std::setw(16) << "Implementation"
              << std::right << std::setw(8) << "Cache"
              << std::setw(10) << "Requests"
              << std::setw(8) << "Hits"
              << std::setw(9) << "Misses"
              << std::setw(11) << "Hit Ratio"
              << std::setw(11) << "Miss Ratio"
              << "    Final Cache\n"
              << std::string(78, '-') << '\n';
}
}

int main()
{
    try {
        const std::filesystem::path testcaseDirectory = "testcases";
        const std::filesystem::path resultsDirectory = "results";
        const std::filesystem::path csvPath = resultsDirectory / "benchmark_results.csv";
        constexpr int benchmarkIterations = 1000;

        Utils::ensureDefaultTestCases(testcaseDirectory);
        Utils::ensureDirectory(resultsDirectory);
        std::ofstream(csvPath, std::ios::trunc).close();

        const auto testCases = Utils::loadTestCases(testcaseDirectory);
        if (testCases.empty()) {
            std::cerr << "No testcases found.\n";
            return 1;
        }

        const Benchmark benchmark(benchmarkIterations);
        printProgramHeader(static_cast<int>(testCases.size()),
                           benchmarkIterations);

        for (const auto& testCase : testCases) {
            auto algorithms = createAlgorithms();
            printTestcaseHeader(testCase);

            if (!benchmark.verifyImplementations(algorithms,
                                                 testCase.requests,
                                                 testCase.cacheSize)) {
                return 1;
            }

            printCorrectnessTableHeader();
            for (const auto& algorithm : algorithms) {
                const SimulationResult result =
                    algorithm->simulateDetailed(testCase.requests,
                                                testCase.cacheSize);
                Utils::printImplementationReport(*algorithm,
                                                 result,
                                                 testCase.cacheSize);
            }
            std::cout << std::string(78, '-') << '\n';

            const auto records = benchmark.run(algorithms,
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
