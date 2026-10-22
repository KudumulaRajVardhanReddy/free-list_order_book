# Order Book & Matching Engine with Custom Memory Pool

A multithreaded order book matching engine implemented in C++20. The system utilizes lock-free concurrency primitives and pre-allocated object pools to ensure zero dynamic memory allocations on the execution path.

## Technical Specifications

### Memory Management
- **`FreeList<Capacity>`**: A custom object pool allocator. Initializes contiguous memory for order structures at startup. Order insertion and deletion fetch and return pointers to this pool in $O(1)$ time, bypassing OS-level `malloc`/`free` or `new`/`delete` calls during runtime.

### Data Structures & Complexity
- **Order Lookup (`std::unordered_map`)**: Maps `OrderId` to `Order*` for $O(1)$ lookup and state verification.
- **Price Levels (`std::map`)**: Bids and asks are managed in ordered trees using `std::greater<Price>` and `std::less<Price>`. Insertion of a new price level is $O(\log N)$.
- **Resting Orders (Intrusive Doubly Linked List)**: Each `PriceLevel` maintains an intrusive doubly linked list of `Order` nodes. This permits $O(1)$ order cancellation and removal from the middle of a queue without traversing the list or requiring secondary pointer allocations.

### Concurrency Model
- **`SPSCRingBuffer<T, Capacity>`**: A Single-Producer Single-Consumer lock-free queue.
- **Memory Ordering**: Uses `std::memory_order_acquire` and `std::memory_order_release` for thread synchronization.
- **Cache Alignment**: Utilizes `alignas` (hardware destructive interference size, typically 64 bytes) on read/write indices to prevent false sharing across CPU cache lines.
- **Thread Decoupling**: An ingestion thread passes by-value `OrderCommand` structs across the ring buffer. The matching engine thread consumes these commands and modifies the order book state sequentially.

## Build Instructions

The project uses CMake and requires a compiler supporting C++20.

```bash
# Generate build files (Ninja or Make)
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..

# Compile the application and benchmarks
cmake --build .
```

## Compiler Flags

The default `CMakeLists.txt` applies the following flags:
`-O3 -Wall -Wextra -Wpedantic -Wno-interference-size`

## Benchmark Results

The benchmark evaluates the single-threaded execution of the engine's hot path: adding a limit order and immediately cancelling it. The test was executed for 1,000,000 iterations.

### Environment
* **Compiler:** GCC 15.2.0 (MinGW-w64)
* **Target:** Release (`-O3`)
* **OS:** Windows x86_64
* **Measurement:** `std::chrono::high_resolution_clock`

### Metrics

| Metric | Value |
| :--- | :--- |
| Operations Executed | 1,000,000 |
| Throughput | 13.29 Million ops/sec |
| Average Latency | 75.23 ns |
| 50th Percentile (Median) | 100 ns |
| 99th Percentile ($p_{99}$) | 100 ns |
| Min Latency | 0 ns* |
| Max Latency | 86.10 µs |

*\* Note on Min Latency: A recorded 0 ns latency is an artifact of the OS timer resolution (Query Performance Counter) being coarser than the L1-cached instruction execution time of the tested hot path.*

## Scope & Future Plans

This project represents my first iteration of a C++20 Order Book & Matching Engine, designed to explore high-throughput, low-latency concurrency models and data structure design. 

While this prototype successfully validates core matching mechanics and lock-free thread decoupling, I am actively working on the next iteration (v2) to address key measurement and architectural limits:

- **Allocation-Free Lookup Tables:** While order nodes use custom pool allocation, containers like `std::map` and `std::unordered_map` still allocate node memory on new inserts. The next iteration replaces these with open-addressing hash tables and fixed-size price level arrays to eliminate all dynamic allocations during runtime.
- **Enhanced Timing Precision:** To bypass OS timer quantization on Windows (`std::chrono` 100ns step resolution), I am moving to batched timing loops and fenced hardware cycle counters (`rdtscp`) to capture micro-latency nuances.
- **Isolated Queue Operations:** Refactoring the micro-benchmark loop to pre-seed resting orders, ensuring test iterations evaluate pure $O(1)$ queue updates rather than repeatedly creating and destroying price levels.
- **Realistic L3 Synthetic Workloads:** Expanding beyond single-order benchmarks by building a multi-level market workload generator to evaluate cross-order fills, order book sweeps, and main-memory cache effects under real load.
