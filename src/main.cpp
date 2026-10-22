#include "orderbook/Types.hpp"
#include "orderbook/Order.hpp"
#include "orderbook/FreeList.hpp"
#include "orderbook/RingBuffer.hpp"
#include "orderbook/PriceLevel.hpp"
#include "orderbook/OrderBook.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace orderbook;

struct OrderCommand {
    enum class Action : uint8_t { Add, Cancel } action;
    OrderId id{0};
    Price price{0};
    Quantity quantity{0};
    Side side{Side::Buy};
    OrderType order_type{OrderType::Limit};
};

int main() {
    std::cout << "\nOrder Book & Matching Engine Pipeline\n\n";

    SPSCRingBuffer<OrderCommand, 1024> command_queue;
    std::atomic<bool> running{true};

    // Single-Threaded Execution
    std::thread engine_thread([&command_queue, &running]() {
        OrderBook<100000> book;
        OrderCommand cmd;

        while (running) {
            if (command_queue.pop(cmd)) {
                if (cmd.action == OrderCommand::Action::Add) {
                    book.add_order(cmd.id, cmd.price, cmd.quantity, cmd.side, cmd.order_type);
                } else if (cmd.action == OrderCommand::Action::Cancel) {
                    book.cancel_order(cmd.id);
                }
            } else {
                std::this_thread::yield();
            }
        }

        while (command_queue.pop(cmd)) {
            if (cmd.action == OrderCommand::Action::Add) {
                book.add_order(cmd.id, cmd.price, cmd.quantity, cmd.side, cmd.order_type);
            } else if (cmd.action == OrderCommand::Action::Cancel) {
                book.cancel_order(cmd.id);
            }
        }

        std::cout << "[Engine] Final Book State size: " << book.size() << " resting order(s).\n";
    });

        // Ingestion Thread Submitting Orders
    std::cout << "Submitting Limit Ask: Order #1 @ $100.50 (Qty: 50)...\n";
    [[maybe_unused]] bool ok = command_queue.push(OrderCommand{OrderCommand::Action::Add, 1, 10050, 50, Side::Sell, OrderType::Limit});
    assert(ok);

    std::cout << "Submitting Limit Ask: Order #2 @ $101.00 (Qty: 100)...\n";
    ok = command_queue.push(OrderCommand{OrderCommand::Action::Add, 2, 10100, 100, Side::Sell, OrderType::Limit});
    assert(ok);

    std::cout << "Submitting Limit Bid: Order #3 @ $100.00 (Qty: 30)...\n";
    ok = command_queue.push(OrderCommand{OrderCommand::Action::Add, 3, 10000, 30, Side::Buy, OrderType::Limit});
    assert(ok);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::cout << "Submitting Crossing Buy: Order #4 @ $100.50 (Qty: 20)...\n";
    ok = command_queue.push(OrderCommand{OrderCommand::Action::Add, 4, 10050, 20, Side::Buy, OrderType::Limit});
    assert(ok);

    std::cout << "Cancelling Order #2...\n";
    ok = command_queue.push(OrderCommand{OrderCommand::Action::Cancel, 2, 0, 0, Side::Sell, OrderType::Limit});
    assert(ok);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    running = false;

    if (engine_thread.joinable()) engine_thread.join();


    std::cout << "Engine execution completed cleanly.\n";
}
