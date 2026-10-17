//Order.hpp

#ifndef ORDERBOOK_ORDER_HPP
#define ORDERBOOK_ORDER_HPP

#include "Types.hpp"

namespace orderbook {

struct Order {
    OrderId id = 0;
    Price price = 0;
    Quantity quantity = 0;
    Side side = Side::Buy;
    OrderType type = OrderType::Limit;

    Order* next = nullptr;
    Order* prev = nullptr;

    Order(
        OrderId order_id, Price order_price, Quantity order_qty,
        Side order_side, OrderType order_type = OrderType::Limit
    ) :
    id(order_id),
    price(order_price),
    quantity(order_qty),
    side(order_side),
    type(order_type),
    next(nullptr),
    prev(nullptr) {}

    Order() = default;
};

}

#endif
