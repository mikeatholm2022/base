// Generic interface of all queue messages and all tpes must be added to this enum
//
// Copyright HOLM, 2023

#pragma once

#include <string>

// =============================================================================================
namespace chaos
{
    // ===========================================================================
    enum QueueType
    {
        QT_Unknown = 0,
        QT_Logger

    };

    // ===========================================================================
    class IQueue
    {
    public:
        IQueue() { m_type = QT_Unknown; }
        virtual ~IQueue() {}

        std::uint32_t get_type() { return m_type; }
        void set_type(std::uint32_t type) { m_type = type; }

        virtual IQueue* clone() { return NULL; }
        virtual std::int32_t get_queue_id() { return -1; }
        virtual std::string to_string() const { return "NOT IMPLEMENTED"; }

    protected:
        std::uint32_t m_type;

    };
}