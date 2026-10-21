// Orderbook.hpp

#include "Types.hpp"
#include "Order.hpp"
#include "FreeList.hpp"
#include "PriceLevel.hpp"
#include <map>
#include <unordered_map>
#include <functional>
#include <cassert>

namespace orderbook {

template <size_t MaxOrders = 100000>
class OrderBook {
public:
    OrderBook() = default;

    bool add_order(OrderId id, Price price, Quantity qty, Side side, OrderType type = OrderType::Limit) noexcept {

        if (order_lookup_.find(id) != order_lookup_.end() || !qty) [[unlikely]] return false;

        Order* incoming = free_list_.allocate();
        if (!incoming) [[unlikely]] return false;

        *incoming = Order(id, price, qty, side, type);
        match_order(incoming);

        if (incoming->quantity > 0 && incoming->type == OrderType::Limit) {
            if (incoming->side == Side::Buy) bids_[incoming->price].push_back(incoming);
            else asks_[incoming->price].push_back(incoming);
            order_lookup_[incoming->id] = incoming;
        }
        else if (!incoming->quantity || incoming->type == OrderType::Market) free_list_.deallocate(incoming);

        return true;
    }

    bool cancel_order(OrderId id) noexcept {
        auto it = order_lookup_.find(id);
        if (it == order_lookup_.end()) [[unlikely]] return false;

        Order* order = it->second;
        order_lookup_.erase(it);

        if (order->side == Side::Buy) {
            auto level_it = bids_.find(order->price);
            if (level_it != bids_.end()) {
                level_it->second.remove(order);

                if (level_it->second.empty()) bids_.erase(level_it);
            }
        }
        else {
            auto level_it = asks_.find(order->price);
            if (level_it != asks_.end()) {
                level_it->second.remove(order);
                if (level_it->second.empty()) asks_.erase(level_it);
            }
        }

        free_list_.deallocate(order);
        return true;
    }

    [[nodiscard]] bool empty() const noexcept { return order_lookup_.empty(); }
    [[nodiscard]] size_t size() const noexcept { return order_lookup_.size(); }

private:

    void match_order(Order* incoming) noexcept {
        if (incoming->side == Side::Buy) match_against_book(incoming, asks_, std::less_equal<Price>{});
        else match_against_book(incoming, bids_, std::greater_equal<Price>{});
    }

    template<typename BookType, typename PriceComparator>
    void match_against_book(Order* incoming, BookType& book, PriceComparator can_match) noexcept {
        auto level_it = book.begin();

        while (level_it != book.end() && incoming->quantity > 0) {
            Price level_price = level_it->first;

            if (incoming->type == OrderType::Limit && !can_match(incoming->price, level_price)) break;

            PriceLevel& level = level_it->second;
            Order* resting = level.head();

            while (resting && incoming->quantity > 0) {
                Order* next_resting = resting->next;

                Quantity fill_qty = std::min(incoming->quantity, resting->quantity);
                incoming->quantity -= fill_qty;
                resting->quantity -= fill_qty;

                if (!resting->quantity) {
                    level.remove(resting);
                    order_lookup_.erase(resting->id);
                    free_list_.deallocate(resting);
                }

                resting = next_resting;
            }

            if (level.empty()) level_it = book.erase(level_it);
            else ++level_it;
        }
    }

private:
    FreeList<MaxOrders> free_list_;

    std::unordered_map<OrderId, Order*> order_lookup_;

    std::map<Price, PriceLevel, std::greater<Price>> bids_;
    std::map<Price, PriceLevel, std::less<Price>> asks_;
};

}
