#include "orderbook/Types.hpp"
#include "orderbook/Order.hpp"
#include "orderbook/FreeList.hpp"
#include "orderbook/PriceLevel.hpp"
#include "orderbook/OrderBook.hpp"

#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>
#include <algorithm>

using namespace orderbook;

void benchmark_add_cancel_hotpath(size_t iterations) {
    OrderBook<100000> book;
    std::vector<double> latencies_ns;
    latencies_ns.reserve(iterations);

    for (size_t i = 1; i <= 1000; ++i) {
        book.add_order(i, 10000, 100, Side::Buy, OrderType::Limit);
        book.cancel_order(i);
    }

    for (size_t i = 1001; i <= 1001 + iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        book.add_order(i, 10000, 100, Side::Buy, OrderType::Limit);
        book.cancel_order(i);

        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::nano> elapsed = end - start;
        latencies_ns.push_back(elapsed.count());
    }

    std::sort(latencies_ns.begin(), latencies_ns.end());

    double sum = std::accumulate(latencies_ns.begin(), latencies_ns.end(), 0.0);
    double avg = sum / iterations;
    double p50 = latencies_ns[static_cast<size_t>(iterations * 0.50)];
    double p99 = latencies_ns[static_cast<size_t>(iterations * 0.99)];
    double min_lat = latencies_ns.front();
    double max_lat = latencies_ns.back();

    std::cout << "=================================================\n";
    std::cout << "   ORDERBOOK HOT-PATH BENCHMARK (" << iterations << " ops)\n";
    std::cout << "=================================================\n";
    std::cout << "  Min Latency:   " << min_lat << " ns\n";
    std::cout << "  Avg Latency:   " << avg << " ns\n";
    std::cout << "  50th %ile:     " << p50 << " ns\n";
    std::cout << "  99th %ile:     " << p99 << " ns\n";
    std::cout << "  Max Latency:   " << max_lat << " ns\n";
    std::cout << "  Throughput:    " << (1000.0 / avg) << " Million ops/sec\n";
    std::cout << "=================================================\n\n";
}

int main() { benchmark_add_cancel_hotpath(1'000'000); }
