// Simple application configuration object that will follow the singleton pattern
//
// Copyright HOLM, 2023

#include "pch.h"
#include "spin_lock.h"
#include "time_utils.h"
#include "application_details.h"

// ==========================================================================================================
namespace chaos
{
    // ======================================================================================================
    ApplicationDetails* ApplicationDetails::m_instance = 0;
    namespace app_det
    {
        chaos::SpinLock logLock;
    }

    // ======================================================================================================
    ApplicationDetails* ApplicationDetails::instance()
    {
        app_det::logLock.lock();
        if (m_instance == 0)
        {
            // Lets create it
            m_instance = new ApplicationDetails();
        }
        app_det::logLock.unlock();

        return m_instance;
    }

    // ======================================================================================================
    ApplicationDetails::ApplicationDetails() :
        m_application_name("unknown"),
        m_application_desc("Generic Chaos App"),
        m_application_inst("UNK"),
        m_application_location("NOT_SET"),
        m_application_version("v0.0.0"),
        m_application_tcp_port(0),
        m_application_udp_port(0),
        m_mothers_address(""),
        m_mothers_port(0)
    {
        std::string temp = chaos::get_environment_string("CHAOS_APPLICATION_NAME");
        if (!temp.empty())
            m_application_name = temp;

        temp = chaos::get_environment_string("CHAOS_APPLICATION_DESC");
        if (!temp.empty())
            m_application_desc = temp;

        temp = chaos::get_environment_string("CHAOS_APPLICATION_INSTANCE");
        if (!temp.empty())
            m_application_inst = temp;

        temp = chaos::get_environment_string("CHAOS_LOCATION");
        if (!temp.empty())
            m_application_location = temp;

        int port = chaos::get_environment_int("CHAOS_APPLICATION_TCP_PORT");
        if (port !=0)
            m_application_tcp_port = port;

        port = chaos::get_environment_int("CHAOS_APPLICATION_UDP_PORT");
        if (port != 0)
            m_application_udp_port = port;

        temp = chaos::get_environment_string("CHAOS_MOTHERS_ADDRESS");
        if (!temp.empty())
            m_mothers_address = temp;

        port = chaos::get_environment_int("CHAOS_MOTHERS_PORT");
        if (port != 0)
            m_mothers_port = port;

        // Lets grab the current time
        m_application_start_time = chaos::time_as_string_posix();
    }

    // ======================================================================================================
    void ApplicationDetails::set_details(const std::string& name, const std::string& desc, const std::string& inst, std::int32_t tcp, std::int32_t udp)
    {
        if (!name.empty())
            m_application_name = name;
        if (!desc.empty())
            m_application_desc = desc;
        if (!inst.empty())
            m_application_inst = inst;
        if (tcp != 0)
            m_application_tcp_port = tcp;
        if (udp != 0)
            m_application_udp_port = udp;
    }

    // ======================================================================================================
    bool ApplicationDetails::is_configuration_okay()
    {
        if (m_mothers_address.empty() || m_mothers_port == 0)
            return false;

        return true;
    }

    // ======================================================================================================
}

