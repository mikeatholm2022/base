// Simple application configuration object that will follow the singleton pattern
//
// Copyright HOLM, 2023

#pragma once

#include "utils.h"

#include <string>

// ===================================================================================================
namespace chaos
{
    // ===============================================================================================
    class ApplicationDetails
    {
    protected:
        ApplicationDetails();

    public:
        ~ApplicationDetails() {}
        static ApplicationDetails* instance();

        void set_details(const std::string& app_name, const std::string& app_desc, const std::string& app_inst, std::int32_t tcp, std::int32_t udp);
        void set_version(const std::string& ver) { m_application_version = ver;  }
        bool is_configuration_okay();

        const std::string& get_application_name() { return m_application_name; }
        const std::string& get_application_description() { return m_application_desc; }
        const std::string& get_application_instance() { return m_application_inst; }
        const std::string& get_application_location() { return m_application_location; }
        const std::string& get_application_version() { return m_application_version; }
        const std::string& get_application_start_time() { return m_application_start_time; }
        const std::string& get_mothers_address() { return m_mothers_address; }
        std::int32_t get_mothers_port() { return m_mothers_port; }
        std::int32_t get_application_tcp_port() { return m_application_tcp_port; }
        std::int32_t get_application_udp_port() { return m_application_udp_port; }

    private:
        static ApplicationDetails*  m_instance;

        std::string     m_application_name;
        std::string     m_application_desc;
        std::string     m_application_inst;
        std::string     m_application_location;
        std::string     m_application_version;
        std::string     m_application_start_time;
        std::int32_t    m_application_tcp_port;
        std::int32_t    m_application_udp_port;
        std::string     m_mothers_address;
        std::int32_t    m_mothers_port;

    };
} // End of namespace