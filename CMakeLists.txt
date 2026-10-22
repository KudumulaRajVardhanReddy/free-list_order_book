# CMakeLists.txt

cmake_minimum_required(VERSION 3.16)
project(LowLevelOrderBook CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags + warning suppressions
add_compile_options(-O3 -Wall -Wextra -Wpedantic -Wno-interference-size)

include_directories(include)

# Main Application Target
add_executable(orderbook_app src/main.cpp)
target_link_libraries(orderbook_app PRIVATE pthread)

# Lightweight Benchmark Target
add_executable(benchmarks_app benchmarks/BenchmarkEngine.cpp)
