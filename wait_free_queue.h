// Simple lock-free object initially published on the boost website
//
// Copyright HOLM, 2023

#pragma once

#include "queue_types.h"

#include <boost/atomic.hpp>
#include <boost/lockfree/queue.hpp>

// =============================================================================================
namespace chaos
{
    // ========================================================================
    template<typename T>
    class WaitFreeQueue
    {
    public:
        struct node
        {
            T data;
            node* next;
        };

        void push(const T& data)
        {
            node* n = new node;
            n->data = data;
            node* stale_head = m_head.load(boost::memory_order_relaxed);
            do {
                n->next = stale_head;
            } while (!m_head.compare_exchange_weak(stale_head, n, boost::memory_order_release));
        }

        node* pop_all(void)
        {
            node* last = pop_all_reverse();
            node* first = 0;
            while (last)
            {
                node* tmp = last;
                last = last->next;
                tmp->next = first;
                first = tmp;
            }

            return first;
        }

        WaitFreeQueue() : m_head(0) {}

        // Alternative interface if ordering is of no importance
        node* pop_all_reverse(void)
        {
            return m_head.exchange(0, boost::memory_order_consume);
        }

    private:
        boost::atomic<node*> m_head;

    };
}