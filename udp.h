// Simple generic UDP object that supports multicast and unicast used for communicating with mother
// The majority of this code comes from internet examples on setting up an async udp boost object.
//
// Copyright HOLM, 2023

#pragma once

// ========================================================================================

#include "udp_messages.h"

#include <boost/cstdint.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>

namespace chaos
{
    // ====================================================================================
    class Udp
    {
    public:
        Udp( boost::asio::io_service& io, const std::string& addr, std::uint16_t port, bool join_multicast_group = true, bool listen = true, const std::string& nic = "" );
        Udp( bool manualStart, boost::asio::io_service& io, const std::string& addr, std::uint16_t port, bool join_multicast_group = true, bool listen = true, const std::string& nic = "");
        virtual ~Udp() {}

        inline std::int32_t get_buffer_size() { return m_buffer_size; }

        void start();
        void shutdown();
        void send_msg( UDP_MSG& pMsg );
        void async_send_msg( UDP_MSG& pMsg );
        
    protected:
        virtual void on_message( const boost::system::error_code& error, size_t bytes_recvd );
        virtual void handle_async_send( boost::shared_ptr<UDP_MSG> msg, const boost::system::error_code& error, std::size_t bytes_transferred );

        void clear_msg(){ memset( &m_msg, 0, sizeof(UDP_MSG) ); }
        void initialize( const std::string& addr, bool joinMulticast, bool listen, const std::string& nic = "" );
        void async_receive();

        UDP_MSG                         m_msg;
        boost::asio::io_service&        m_main_io;
        boost::asio::ip::udp::endpoint  m_endpoint;
        boost::asio::ip::udp::endpoint  m_remote_endpoint;
        boost::asio::ip::udp::socket    m_socket;
        boost::asio::ip::udp::socket    m_remote_socket;
        std::string                     m_nic;
        std::string                     m_addr;
        boost::uint16_t                 m_port;
        boost::int32_t                  m_buffer_size;
        bool                            m_multicast_flag;
        bool                            m_listen;

    };

    // ====================================================================================

} // End of namespace
