// Simple logger and event posting object
//
// Copyright HOLM, 2023

#include "pch.h"
#include "logger.h"
#include "utils.h"
#include "spin_lock.h"
#include "wait_free_queue.h"
#include "time_utils.h"
#include "application_details.h"

#ifndef WIN32
#include <unistd.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#else
#include <direct.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <iomanip>

// ========================================================================================================
namespace chaos
{
    // ====================================================================================================
    const char* get_state(int state)
    {
        switch (state)
        {
        case FAT_MSG:
            return "FAT";

        case ERR_MSG:
            return "ERR";

        case PUB_MSG:
            return "PUB";

        case INF_MSG:
            return "INF";

        case WARN_MSG:
            return "WARN";

        case DEB_MSG:
            return "DEB";

        default:
            return "UNK";
        }
    }

    // ======================================================================================================
    Logger* Logger::m_instance = 0;
    namespace logger
    {
        chaos::SpinLock logLock;
    }

    // ======================================================================================================
    Logger* Logger::instance()
    {
        logger::logLock.lock();
        if (m_instance == 0)
        {
            // Lets create it
            m_instance = new Logger();
        }
        logger::logLock.unlock();

        return m_instance;
    }

    // ======================================================================================================
    Logger::Logger() :
        m_thread(NULL),
        m_timer(m_log_io, boost::posix_time::seconds(1)),
        m_strand(m_log_io),
        m_application_name(chaos::ApplicationDetails::instance()->get_application_name()),
        m_mother(NULL),
        m_heartbeat_counter(0),
        m_shutdown(false)
    {
        std::string dir = chaos::get_environment_string("LOG_DIRECTORY");
        if (dir.empty())
            dir = "logs";

        std::string temp = chaos::get_environment_string("APPLICATION_NAME");
        if (!temp.empty())
            m_application_name = temp;

        // First lets create the log file
        std::string file = dir + "/" + m_application_name + "_" + chaos::time_as_string("%Y%m%d_%H%M%S") + ".log";
        struct stat st;
        if (stat(dir.c_str(), &st) == 0)
        {
            // The directory exist so create the file and open it to write
            m_log_stream.open(file.c_str(), std::ios_base::out);
            if (!m_log_stream.is_open())
            {
                std::cerr << "Failed to create log file ... " << file << std::endl;
            }
            else
            {
                std::streambuf* psbuf = m_log_stream.rdbuf();
                std::cout.rdbuf(psbuf);
                std::cerr.rdbuf(psbuf);
            }
        }
        else
        {
            // The directory doesn't exist so create it first
#ifndef WIN32
            if (mkdir(dir.c_str(), 0755) == 0)
#else
            if (_mkdir(dir.c_str()) == 0)
#endif
            {
                m_log_stream.open(file.c_str(), std::ios_base::out);
                if (!m_log_stream.is_open())
                {
                    std::cerr << "Failed to create log file ... " << file << std::endl;
                }
                else
                {
                    std::streambuf* psbuf = m_log_stream.rdbuf();
                    std::cout.rdbuf(psbuf);
                    std::cerr.rdbuf(psbuf);
                }
            }
            else
            {
                std::cerr << "Failed to create log directory 2 ... " << dir << std::endl;
            }
        }

        m_thread = new boost::thread(&Logger::log_io_thread, this);
        m_timer.async_wait(m_strand.wrap(boost::bind(&Logger::write_to_file, this)));

        auto app_desc = chaos::ApplicationDetails::instance();
        // We will only be publishing messages from this object
        m_mother = new chaos::Udp(m_log_io, app_desc->get_mothers_address(), app_desc->get_mothers_port(), false, false);

        // The message object for the mother heartbeat
        m_mother_msg.clear();
        m_mother_msg.header.m_type = chaos::MessageTypes::MOTHER;
        m_mother_msg.header.m_length = m_mother_msg.mother.getLength();
        strcpy(m_mother_msg.mother.m_application, m_application_name.c_str());
        strcpy(m_mother_msg.mother.m_description, app_desc->get_application_description().c_str());
        strcpy(m_mother_msg.mother.m_location, app_desc->get_application_location().c_str());
        strcpy(m_mother_msg.mother.m_instance, app_desc->get_application_name().c_str());
        strcpy(m_mother_msg.mother.m_host, boost::asio::ip::host_name().c_str());
        strcpy(m_mother_msg.mother.m_start_time, app_desc->get_application_start_time().c_str());
        strcpy(m_mother_msg.mother.m_version, app_desc->get_application_version().c_str());
        m_mother_msg.mother.m_pid = chaos::get_pid();
        m_mother_msg.mother.m_tcp_pid = app_desc->get_application_tcp_port();
        m_mother_msg.mother.m_udp_port = app_desc->get_application_tcp_port();

        // The message object for the event log
        m_log_msg.clear();
        m_log_msg.header.m_type = chaos::MessageTypes::LOG;
        m_log_msg.header.m_length = m_mother_msg.log.getLength();
        strcpy(m_log_msg.log.m_application, m_application_name.c_str());
        strcpy(m_log_msg.log.m_instance, app_desc->get_application_name().c_str());
        strcpy(m_log_msg.log.m_host, boost::asio::ip::host_name().c_str());
    }

    // ======================================================================================================
    void Logger::log_io_thread()
    {
        chaos::set_thread_name("Logger");
        std::stringstream ss;
        ss << "Logger Thread = " << boost::this_thread::get_id();
        LOG(PUB_MSG, ss.str().c_str());

        boost::asio::io_service::work work(m_log_io);
        m_log_io.run();
    }

    // ======================================================================================================
    Logger::~Logger()
    {
        m_log_io.stop();
        chaos::sleep(1);

        if (m_thread)
        {
            delete m_thread;
            m_thread = NULL;
        }

        if (m_log_stream.is_open())
            m_log_stream.close();
    }

    // ======================================================================================================
    void Logger::log_information(std::int32_t state, const std::string& msg, const std::string& file, std::int32_t line)
    {
        LogMessage* data = new LogMessage(state, msg, file, line);
        m_queue.push(data);
    }

    // ======================================================================================================
    void Logger::write_to_file()
    {
        chaos::WaitFreeQueue<chaos::IQueue*>::node* x = m_queue.pop_all();
        while (x)
        {
            chaos::WaitFreeQueue<chaos::IQueue*>::node* tmp = x;

            switch (tmp->data->get_type())
            {
                case chaos::QT_Logger:
                {
                    // This is a normal log message
                    LogMessage* lm = dynamic_cast<LogMessage*>(tmp->data);
                    if (lm)
                    {
                        if (m_log_stream.is_open() && are_we_running_in_normal_mode())
                        {
                            try
                            {
                                std::stringstream ss;
                                ss << chaos::time_in_micros(lm->m_now) << " [" << get_state(lm->m_state) << "][" << lm->m_id << "] " << lm->m_msg << "  [" << lm->m_file << ":" << lm->m_line << "]\n";
                                m_log_stream << ss.str();

                                // Lets publish the FATAL, ERROR and PUBLIC messages to Mother
                                if (lm->m_state <= PUB_MSG)
                                    publish_log_message(lm->m_state, lm->m_msg);
                            }
                            catch (...)
                            {
                                // When we are shutting down we could be in this conditional statement but we want to exit out of it
                                break;
                            }
                        }
                    }
                }
                break;

            }

            x = x->next;
            // Clean-up time
            delete tmp->data;
            delete tmp;
        }

        if (m_log_stream.is_open() && are_we_running_in_normal_mode())
        {
            m_log_stream.flush();

            // Lets send the mother heartbeat every 5 seconds
            publish_mothers_heartbeat();

            // Now lets sleep for a second before processing these messages again
            m_timer.expires_at(m_timer.expires_at() + boost::posix_time::seconds(1));
            m_timer.async_wait(m_strand.wrap(boost::bind(&Logger::write_to_file, this)));
        }
        else
        {
            std::cerr << "PUBLIC - logging thread is shutting down" << std::endl;
        }
    }

    // ======================================================================================================
    void Logger::create_if_doesnt_exist(const std::string& dir)
    {
        struct stat st;
        if (stat(dir.c_str(), &st) != 0)
        {
            // The directory doesn't exist so create it first
#ifndef WIN32
            if (mkdir(dir.c_str(), 0755) != 0)
#else
            if (_mkdir(dir.c_str()) != 0)
#endif
            {
                std::cerr << "Failed to create data directory ... " << dir << std::endl;
            }
        }
    }

    // ======================================================================================================
    void Logger::publish_log_message(int state, const std::string& msg)
    {
        if (m_mother)
        {
            m_log_msg.log.clear_log_message();

            strcpy(m_log_msg.log.m_severity, get_state(state));
            // 1024 is the maximum length we can send
            if (msg.size() <= 1023)
                strcpy(m_log_msg.log.m_log_message, msg.c_str());
            else
                strncpy(m_log_msg.log.m_log_message, msg.c_str(), 1023);

            m_mother->send_msg(m_log_msg);
        }
    }
    
    // ======================================================================================================
    void Logger::publish_mothers_heartbeat()
    {
        // Send the ping every 5 seconds
        if (m_mother && (++m_heartbeat_counter % 5 == 0))
        {
            // The version can update while the application is running so we need to update this as well
            memset(m_mother_msg.mother.m_version, 0, sizeof(m_mother_msg.mother.m_version));
            strcpy(m_mother_msg.mother.m_version, chaos::ApplicationDetails::instance()->get_application_version().c_str());
            m_mother->send_msg(m_mother_msg);
        }
    }

    // ======================================================================================================
}

