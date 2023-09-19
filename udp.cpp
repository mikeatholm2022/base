// Simple generic UDP object that supports multicast and unicast used for communicating with mother.
// The majority of this code comes from internet examples on setting up an async udp boost object.
//
// Copyright HOLM, 2023

#include "pch.h"
#include "udp.h"

#include <cstdlib>

#include <boost/bind/bind.hpp>

// ========================================================================
namespace chaos
{
    // ========================================================================

#ifdef WIN32
    WSADATA  WSAData;
#endif

    // ========================================================================
    Udp::Udp( boost::asio::io_service& io, const std::string& addr, std::uint16_t port, bool join_multicast_group, bool listen, const std::string& nic ) :
        m_main_io(io),
        m_endpoint( boost::asio::ip::address::from_string( addr ), port ),
        m_remote_endpoint( boost::asio::ip::address::from_string( addr ), port ),
        m_socket(io),
        m_remote_socket(io, m_remote_endpoint.protocol()),
        m_port(port),
        m_multicast_flag(join_multicast_group)
    {
      // Lets set the default receive buffer size to 30MB
      int size = 30 * 1024 * 1024;
      m_remote_socket.set_option( boost::asio::ip::udp::socket::send_buffer_size(size) );
      initialize(addr, join_multicast_group, listen, nic);
    }

    // ========================================================================
    Udp::Udp( bool manualStart, boost::asio::io_service& io, const std::string& addr, std::uint16_t port, bool join_multicast_group, bool listen, const std::string& nic ) :
        m_main_io(io),
        m_endpoint(boost::asio::ip::address::from_string(addr), port),
        m_remote_endpoint(boost::asio::ip::address::from_string(addr), port),
        m_socket(io),
        m_remote_socket(io, m_remote_endpoint.protocol()),
        m_nic(nic),
        m_addr(addr),
        m_port(port),
        m_multicast_flag(join_multicast_group),
        m_listen(listen)
    {
      // Lets set the default receive buffer size to 30MB
      int size = 30 * 1024 * 1024;
      m_remote_socket.set_option( boost::asio::ip::udp::socket::send_buffer_size(size) );
      if( !manualStart )
        initialize(addr, join_multicast_group, listen, nic);
    }

    // ========================================================================
    void Udp::start()
    {
        initialize( m_addr, m_multicast_flag, m_listen, m_nic );
    }

    // ========================================================================
    void Udp::initialize( const std::string& addr, bool join_multicast_group, bool listen, const std::string& nic )
    {
        if( listen )
        {
            boost::asio::ip::address listen_address = boost::asio::ip::address::from_string( "0.0.0.0" );
            boost::asio::ip::address_v4 multicast_address = boost::asio::ip::address_v4::from_string( addr );

            // Create the socket so that multiple may be bound to the same address.
            boost::asio::ip::udp::endpoint listen_endpoint( listen_address, m_port );
            m_socket.open( listen_endpoint.protocol());
            // We want several applications to be able to use the address
            // NOTE - that if we are receiving unicast messages then only one application on a server will receive them
            m_socket.set_option( boost::asio::ip::udp::socket::reuse_address(true) );
            // Lets set the default receive buffer size to 30MB
            int size = 30 * 1024 * 1024;
            m_socket.set_option( boost::asio::ip::udp::socket::receive_buffer_size(size) );
            boost::asio::ip::udp::socket::receive_buffer_size option;
            m_socket.get_option( option );
            m_buffer_size = option.value();
            // Lets set the default send buffer size
            m_socket.set_option( boost::asio::ip::udp::socket::send_buffer_size(size) );
            // Now we listen
            m_socket.bind( listen_endpoint );
            // Set the loop-back for multicast, this will stop us from receiving our own messages
            m_socket.set_option( boost::asio::ip::multicast::enable_loopback( true ) );
            // Set the Time-to-Live of the multicast, this should be high if we are sending messages over a large WAN
            m_socket.set_option( boost::asio::ip::multicast::hops(20) );

            // Join the multicast group.
            if (join_multicast_group)
            {
                if( nic.empty() )
                    m_socket.set_option( boost::asio::ip::multicast::join_group(multicast_address) );
                else
                {
                    boost::asio::ip::address_v4 local_nic = boost::asio::ip::address_v4::from_string( nic );
                    m_socket.set_option( boost::asio::ip::multicast::join_group(multicast_address, local_nic) );
                }
            }
            async_receive();
        }
    }

    // ========================================================================
    void Udp::handle_async_send( boost::shared_ptr<UDP_MSG> msg, const boost::system::error_code& error, std::size_t bytes_transferred )
    {
        // Will get called after the message is published
        //std::cout << "handle_async_send " << error << ": " << bytes_transferred << std::endl;
    }

    // ========================================================================
    void Udp::send_msg( UDP_MSG& msg )
    {
        m_remote_socket.send_to( boost::asio::buffer((char*)&(msg), msg.header.m_length), m_remote_endpoint );
    }

    // ========================================================================
    void Udp::async_send_msg( UDP_MSG& msg )
    {
        PUDP_MSG pmsg = new UDP_MSG;
        memcpy( pmsg, &msg, sizeof(UDP_MSG) );
        // Will delete the object when its done
        boost::shared_ptr<UDP_MSG> obj(pmsg);
        m_remote_socket.async_send_to( boost::asio::buffer((char*)pmsg, msg.header.m_length), m_remote_endpoint,
                                        boost::bind(&Udp::handle_async_send, this, obj, boost::asio::placeholders::error, 
                                        boost::asio::placeholders::bytes_transferred) );
    }

    // ========================================================================
    void Udp::async_receive()
    {
        clear_msg();
        m_socket.async_receive_from(boost::asio::buffer(&(m_msg), sizeof(UDP_MSG)), m_endpoint,
                                    boost::bind(&Udp::on_message, this,
                                    boost::asio::placeholders::error,
                                    boost::asio::placeholders::bytes_transferred ) );
    }

    // ========================================================================
    void Udp::on_message( const boost::system::error_code& error, size_t bytes_recvd )
    {
        // Parse the message or do something with it.
        // This is typically the only method that will be overridden

        // Listen again
        async_receive();
    }

    // ========================================================================
    void Udp::shutdown()
    {
        if( m_socket.is_open() )
        {
            m_socket.cancel();
            m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            m_socket.close();
        }

        if( m_remote_socket.is_open() )
        {
            m_remote_socket.cancel();
            m_remote_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            m_remote_socket.close();
        }
    }
} // End of namespace
