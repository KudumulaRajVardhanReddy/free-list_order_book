// BenchmarkEngine.cpp

#include <benchmark/benchmark.h>
#include "orderbook/OrderBook.hpp"

using namespace orderbook;

static void BM_OrderBookAddCancel(benchmark::State& state) {
    OrderBook<100000> book;
    OrderId id = 1;

    for (auto _ : state) {
        book.add_order(id, 10000, 100, Side::Buy, OrderType::Limit);
        book.cancel_order(id);
        ++id;
    }
}
BENCHMARK(BM_OrderBookAddCancel);

BENCHMARK_MAIN();
