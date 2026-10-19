// FreeList.hpp

#pragma once
#include "Order.hpp"
#include <memory>
#include <cstddef>
#include <cassert>

namespace orderbook {

template<size_t Capacity>
class FreeList {
public:
    FreeList() {
        m_pool = std::make_unique<Order[]>(Capacity);

        // Create LinkedList : Link 'i'th order to 'i+1'th order.
        for (size_t i = 0; i < Capacity - 1; ++i) m_pool[i].next = &m_pool[i + 1];
        m_pool[Capacity - 1].next = nullptr;

        m_head = &m_pool[0];
    }

    // Block operations on l-values.
    FreeList(const FreeList&) = delete;
    FreeList& operator=(const FreeList&) = delete;

    // Block operations on r-values.
    FreeList(FreeList&&) = delete;
    FreeList& operator=(FreeList&&) = delete;

    [[nodiscard]] Order* allocate() noexcept {
        // Check if the pool is out of memory.
        if (m_head == nullptr) [[unlikely]] return nullptr;

        Order* node = m_head;
        m_head = m_head->next;

        node->next = nullptr;
        node->prev = nullptr;

        return node;
    }

    void deallocate(Order* node) noexcept {
        assert(node != nullptr);

         // Check if the order is in the pool range.
        assert(node >= &m_pool[0] && node < &m_pool[Capacity]);

        node->next = m_head;
        node->prev = nullptr;

        m_head = node;
    }

    [[nodiscard]] constexpr size_t capacity() const noexcept { return Capacity; }

private:
    std::unique_ptr<Order[]> m_pool{nullptr};
    Order* m_head{nullptr};
};

}
