//Order.hpp

#pragma once
#include "Types.hpp"

namespace orderbook {

struct Order {
    OrderId id{0};
    Price price{0};
    Quantity quantity{0};
    Side side{Side::Buy};
    OrderType type{OrderType::Limit};

    Order* next{nullptr};
    Order* prev{nullptr};

    Order(
        OrderId id, Price price, Quantity qty,
        Side side, OrderType type = OrderType::Limit
    ) :
    id(id),
    price(price),
    quantity(qty),
    side(side),
    type(type) {}

    Order() = default;
};

}
