// Types.hpp

#pragma once
#include <cstdint>

namespace orderbook {

    // Using Fixed Point Integers to prevent precision
    // issues that arise from floats and doubles.

    using OrderId = uint64_t;
    using Price = uint64_t;
    using Quantity = uint32_t;

    enum class Side : uint8_t {
        Buy = 0,
        Sell = 1
    };

    enum class OrderType : uint8_t {
        Limit = 0,
        Market = 1
    };
}
