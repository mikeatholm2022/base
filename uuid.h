// Generic uuid object
//
// Copyright HOLM, 2023

#pragma once

#include <cstring>
#include <string>

#include <uuid/uuid.h>

// ========================================================================
namespace chaos
{
    // ========================================================================
    static void generate_uuid( uuid_t& id )
    {
        uuid_generate( id );
    }

    // ========================================================================
    static void generate_time_safe_uuid( uuid_t& id )
    {
        uuid_generate_time_safe( id );
    }

    // ========================================================================
    static void generate_time_uuid( uuid_t& id )
    {
        uuid_generate_time( id );
    }

    // ========================================================================
    static std::string convert_uuid_to_string( uuid_t& id )
    {
        char uuid_str[37];
        memset( uuid_str, 0, sizeof(uuid_str) );
        uuid_unparse_lower( id, uuid_str );
        return std::string( uuid_str );
    }

    // ========================================================================
    static void convert_uuid_from_string( const std::string& str, uuid_t& id )
    {
        uuid_parse( str.c_str(), id );
    }

    // ========================================================================
    static bool compare_uuid( uuid_t& id1, uuid_t& id2 )
    {
        if( uuid_compare( id1, id2 ) == 0 )
            return true;
        else
            return false;
    }

    // ========================================================================
    static std::string generate_uuid_string()
    {
        uuid_t id;
        uuid_generate_time_safe( id );
        return convert_uuid_to_string( id );
    }

}
