#include "Utils.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <unordered_set>

int SimulationResult::totalRequests() const
{
    return hits + misses;
}

double SimulationResult::hitRatio() const
{
    const int total = totalRequests();
    return total == 0 ? 0.0 : static_cast<double>(hits) / total;
}

double SimulationResult::missRatio() const
{
    const int total = totalRequests();
    return total == 0 ? 0.0 : static_cast<double>(misses) / total;
}

namespace Utils
{
std::vector<int> generateRandomRequests(int requestCount,
                                        int pageRange,
                                        unsigned int seed)
{
    if (requestCount < 0) {
        throw std::invalid_argument("requestCount must be non-negative");
    }
    if (pageRange <= 0) {
        throw std::invalid_argument("pageRange must be positive");
    }

    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> distribution(0, pageRange - 1);

    std::vector<int> requests;
    requests.reserve(static_cast<std::size_t>(requestCount));
    for (int i = 0; i < requestCount; ++i) {
        requests.push_back(distribution(generator));
    }
    return requests;
}

void writeTestCase(const std::filesystem::path& path,
                   int cacheSize,
                   const std::vector<int>& requests)
{
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Unable to write testcase: " + path.string());
    }

    output << cacheSize << ' ' << requests.size() << '\n';
    for (std::size_t i = 0; i < requests.size(); ++i) {
        output << requests[i] << (i + 1 == requests.size() ? '\n' : ' ');
    }
}

TestCase readTestCase(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Unable to read testcase: " + path.string());
    }

    TestCase testCase;
    testCase.name = path.filename().string();
    testCase.path = path;

    int requestCount = 0;
    input >> testCase.cacheSize >> requestCount;
    if (!input || testCase.cacheSize < 0 || requestCount < 0) {
        throw std::runtime_error("Invalid testcase header: " + path.string());
    }

    testCase.requests.reserve(static_cast<std::size_t>(requestCount));
    for (int i = 0; i < requestCount; ++i) {
        int page = 0;
        input >> page;
        if (!input) {
            throw std::runtime_error("Invalid request sequence: " + path.string());
        }
        testCase.requests.push_back(page);
    }

    return testCase;
}

std::vector<TestCase> loadTestCases(const std::filesystem::path& directory)
{
    std::vector<TestCase> testCases;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            testCases.push_back(readTestCase(entry.path()));
        }
    }

    std::ranges::sort(testCases, [](const TestCase& lhs, const TestCase& rhs) {
        if (lhs.cacheSize != rhs.cacheSize) {
            return lhs.cacheSize < rhs.cacheSize;
        }
        if (lhs.requests.size() != rhs.requests.size()) {
            return lhs.requests.size() < rhs.requests.size();
        }
        return lhs.name < rhs.name;
    });
    return testCases;
}

void ensureDefaultTestCases(const std::filesystem::path& directory)
{
    ensureDirectory(directory);

    const std::vector<int> cacheSizes {3, 4, 5};
    const std::vector<int> requestCounts {10, 50, 100};

    for (const int cacheSize : cacheSizes) {
        for (const int requestCount : requestCounts) {
            const std::filesystem::path path =
                directory / ("cache" + std::to_string(cacheSize) + "_" +
                             std::to_string(requestCount) + ".txt");
            if (std::filesystem::exists(path)) {
                continue;
            }

            constexpr int pageRange = 10;
            const unsigned int seed =
                static_cast<unsigned int>(cacheSize * 10'000 + requestCount);
            writeTestCase(path,
                          cacheSize,
                          generateRandomRequests(requestCount, pageRange, seed));
        }
    }
}

void ensureDirectory(const std::filesystem::path& directory)
{
    std::filesystem::create_directories(directory);
}

std::vector<int> sortedCache(const std::vector<int>& cache)
{
    std::vector<int> sorted = cache;
    std::ranges::sort(sorted);
    return sorted;
}

std::vector<int> sortedCacheFromSet(const std::unordered_set<int>& cache)
{
    std::vector<int> sorted(cache.begin(), cache.end());
    std::ranges::sort(sorted);
    return sorted;
}

bool sameSimulationResult(const SimulationResult& lhs,
                          const SimulationResult& rhs)
{
    return lhs.hits == rhs.hits &&
           lhs.misses == rhs.misses &&
           lhs.finalCache == rhs.finalCache;
}

void printVector(std::ostream& output, const std::vector<int>& values)
{
    output << '[';
    for (std::size_t i = 0; i < values.size(); ++i) {
        output << values[i];
        if (i + 1 != values.size()) {
            output << ", ";
        }
    }
    output << ']';
}

void printImplementationReport(const BeladyAlgorithm& algorithm,
                               const SimulationResult& result,
                               int cacheSize)
{
    std::cout << std::left << std::setw(16) << algorithm.name()
              << std::right << std::setw(8) << cacheSize
              << std::setw(10) << result.totalRequests()
              << std::setw(8) << result.hits
              << std::setw(9) << result.misses
              << std::setw(11) << std::fixed << std::setprecision(4)
              << result.hitRatio()
              << std::setw(11) << result.missRatio()
              << "    ";
    printVector(std::cout, result.finalCache);
    std::cout << '\n';
}
}
