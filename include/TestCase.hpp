#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct TestCase
{
    std::string name;
    int cacheSize = 0;
    std::vector<int> requests;
    std::filesystem::path path;
};
