cmake_minimum_required(VERSION 3.20)

project(OptimalCaching LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(optimal_caching
    src/BeladyNaive.cpp
    src/BeladyHeap.cpp
    src/BeladySet.cpp
    src/Benchmark.cpp
    src/Utils.cpp
    src/main.cpp
)

target_include_directories(optimal_caching PRIVATE include)

if (MSVC)
    target_compile_options(optimal_caching PRIVATE /W4)
else()
    target_compile_options(optimal_caching PRIVATE -Wall -Wextra -Wpedantic)
endif()
