// Simple logger and event posting object
//
// Copyright HOLM, 2023

#pragma once

#include "utils.h"
#include "queue_types.h"
#include "wait_free_queue.h"
#include "time_utils.h"
#include "udp_messages.h"
#include "udp.h"

#include <map>

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/thread/thread.hpp>


// =================================================================================

#define FAT_MSG         0
#define ERR_MSG         1
#define PUB_MSG         2
#define INF_MSG         3
#define WARN_MSG	    4
#define DEB_MSG         5

#define LOG( X, Y )     chaos::Logger::instance()->log_information( X, Y, __FILE__, __LINE__ );
#define SPACE           std::string(" ")
#define STR( X )        std::string(X)

// ===================================================================================================
namespace chaos
{
    // ===============================================================================================
    class LogMessage : public chaos::IQueue
    {
    public:
        LogMessage() :
            m_state(0),
            m_msg(""),
            m_file(""),
            m_line(0),
            m_id(boost::this_thread::get_id()),
            m_now(boost::posix_time::microsec_clock::universal_time())
        {
            set_type(chaos::QT_Logger);
        }

        LogMessage(std::int32_t s, const std::string& m, const std::string& f, std::int32_t l) :
            m_state(s),
            m_msg(m),
            m_file(f),
            m_line(l),
            m_id(boost::this_thread::get_id()),
            m_now(boost::posix_time::microsec_clock::universal_time())
        {
            set_type(chaos::QT_Logger);
        }

        std::int32_t    m_state;
        std::string     m_msg;
        std::string     m_file;
        std::int32_t    m_line;

        boost::thread::id           m_id;
        boost::posix_time::ptime    m_now;

    };

    // =============================================================================
    class Logger
    {
    protected:
        Logger();

    public:
        virtual ~Logger();
        static Logger* instance();

        void log_information(std::int32_t state, const std::string& msg, const std::string& file, std::int32_t line);
        void stop() 
        { 
            m_shutdown = true;
            m_log_io.stop();
        }

    private:
        void write_to_file();
        void log_io_thread();
        void create_if_doesnt_exist(const std::string& dir);
        void publish_log_message(int state, const std::string& msg);
        void publish_mothers_heartbeat();
        bool are_we_running_in_normal_mode() { return !m_shutdown;  }

    private:
        static Logger*                          m_instance;
        chaos::WaitFreeQueue<chaos::IQueue*>    m_queue;
        boost::thread*                          m_thread;
        boost::asio::io_service                 m_log_io;
        boost::asio::deadline_timer             m_timer;
        boost::asio::io_service::strand         m_strand;
        std::uint32_t                           m_log_level;
        std::ofstream                           m_log_stream;
        std::string                             m_application_name;
        chaos::UDP_MSG                          m_mother_msg;
        chaos::UDP_MSG                          m_log_msg;
        chaos::Udp*                             m_mother;
        std::int32_t                            m_heartbeat_counter;
        volatile bool                           m_shutdown;

    };
} // End of namespace