#pragma once

#include "BeladyAlgorithm.hpp"
#include "TestCase.hpp"

#include <filesystem>
#include <iosfwd>
#include <unordered_set>
#include <vector>

namespace Utils
{
constexpr int infinityIndex = 1'000'000'000;

std::vector<int> generateRandomRequests(int requestCount,
                                        int pageRange,
                                        unsigned int seed);

void writeTestCase(const std::filesystem::path& path,
                   int cacheSize,
                   const std::vector<int>& requests);

TestCase readTestCase(const std::filesystem::path& path);
std::vector<TestCase> loadTestCases(const std::filesystem::path& directory);

void ensureDefaultTestCases(const std::filesystem::path& directory);
void ensureDirectory(const std::filesystem::path& directory);

std::vector<int> sortedCache(const std::vector<int>& cache);
std::vector<int> sortedCacheFromSet(const std::unordered_set<int>& cache);

bool sameSimulationResult(const SimulationResult& lhs,
                          const SimulationResult& rhs);

void printVector(std::ostream& output, const std::vector<int>& values);
void printImplementationReport(const BeladyAlgorithm& algorithm,
                               const SimulationResult& result,
                               int cacheSize);
}
