//FreeList.hpp

#ifndef ORDERBOOK_FREELIST_HPP
#define ORDERBOOK_FREELIST_HPP

#include "Order.hpp"
#include <vector>
#include <cstddef>
#include <cassert>

namespace orderbook {

template <size_t Capacity>
class FreeList {
public:
    FreeList() {
        m_pool.resize(Capacity);

        for (size_t i = 0; i < Capacity - 1; ++i) m_pool[i].next = &m_pool[i + 1];
        m_pool[Capacity - 1].next = nullptr;

        m_head = &m_pool[0];
    }

    FreeList(const FreeList&) = delete;
    FreeList& operator=(const FreeList&) = delete;

    Order* allocate() {
        if (m_head == nullptr) [[unlikely]] { return nullptr; }

        Order* node = m_head;
        m_head = m_head->next;

        node->next = nullptr;
        node->prev = nullptr;

        return node;
    }

    void deallocate(Order* node) {
        assert(node != nullptr);

        node->next = m_head;
        node->prev = nullptr;
        m_head = node;
    }

    [[nodiscard]] size_t capacity() const noexcept { return Capacity; }

private:
    std::vector<Order> m_pool;
    Order* m_head{nullptr};
};

}

#endif
