// Generic UDP messages that will be used in conjuction with mother
//
// Copyright HOLM, 2023

#pragma once

#include <cstring>
#include <iostream>

namespace chaos
{
    // ============================================================================================
    #pragma pack(1)

    // Message Types
    enum MessageTypes
    {
        UNDEFINED,
        MOTHER,
        LOG,
        SDM_SNAPSHOT,
        SDM_EVENT,
        SDM
    };

    #define     SIZE_8        8
    #define     SIZE_40       40
    #define     SIZE_80       80
    #define     SIZE_200      200
    #define     SIZE_1024     1024
    #define     SIZE_1400     1400

    // ============================================================================================
    typedef struct _MSG_HEADER
    {
        int   m_length;
        int   m_seq;
        int   m_ref;
        int   m_mask; // 0-Default, 1-Forwarded, 2-Compresssed, 4-Encrypted, 8-Sequence Number Reset
        char  m_type;

        _MSG_HEADER() 
        {
            clear();
        }

        inline void clear() 
        {
            m_length = m_seq = m_ref = m_mask = 0;
            m_type = 0;
        }

        // SDM = self describing message
        inline bool is_SDM() 
        {
            switch(m_type)
            {
                case SDM:
                case SDM_SNAPSHOT:
                case SDM_EVENT:
                    return true;
                default:
                    return false;
            }
        }

        inline int getLength() { return sizeof(_MSG_HEADER); }
        inline bool is_Forwarded() { return (m_mask & 1) ? true : false; } 

    } MSG_HEADER, PMSG_HEADER;

    // ============================================================================================
    typedef struct _MSG_LOG
    {
        char  m_severity[SIZE_8];
        char  m_application[SIZE_80];
        char  m_instance[SIZE_80];
        char  m_host[SIZE_80];
        char  m_log_message[SIZE_1024];

        inline void clear_log_message()
        {
            memset(&m_severity, 0, sizeof(m_severity));
            memset(&m_log_message, 0, sizeof(m_log_message));
        }

        inline int getLength() { return sizeof(MSG_HEADER)+sizeof(_MSG_LOG); }

    } MSG_LOG, *PMSG_LOG;

    // ============================================================================================
    typedef struct _MSG_MOTHER
    {
        char  m_application[SIZE_80];
        char  m_description[SIZE_200];
        char  m_location[SIZE_40];
        char  m_instance[SIZE_80];
        char  m_host[SIZE_80];
        char  m_start_time[SIZE_40];
        char  m_version[SIZE_40];
        int   m_pid;
        int   m_tcp_pid;
        int   m_udp_port;
        
        inline int getLength() { return sizeof(MSG_HEADER)+sizeof(_MSG_MOTHER); }

    } MSG_MOTHER, *PMSG_MOTHER;

    // ============================================================================================
    typedef struct _MSG_SDM
    {
        char  m_data[SIZE_1400];

    } MSG_SDM, *PMSG_SDM;

    // ============================================================================================
    typedef struct _UDP_MSG
    {
        MSG_HEADER  header;
        union
        {
            MSG_LOG     log;
            MSG_MOTHER  mother;
            MSG_SDM     sdm;
        };

        inline void clear() 
        {
            header.clear();
            // SDM is always the largest message which is why we choose it
            memset( &sdm, 0, sizeof(MSG_SDM) );
        }

    } UDP_MSG, *PUDP_MSG;

    // ============================================================================================

#pragma pack()

} // End of namespace 