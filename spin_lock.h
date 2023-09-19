// Simple spin lock implementation
//
// Copyright HOLM, 2023

#pragma once

#include <boost/atomic.hpp>

// =============================================================================================
namespace chaos
{
    // =========================================================================================
    class SpinLock
    {
    private:
        typedef enum { Locked, Unlocked } LockState;
        boost::atomic<LockState> m_state;

    public:
        SpinLock()
            : m_state(Unlocked)
        {}

        void lock()
        {
            while (m_state.exchange(Locked, boost::memory_order_acquire) == Locked) { /* busy-wait */ }
        }

        void unlock()
        {
            m_state.store(Unlocked, boost::memory_order_release);
        }

    };
}