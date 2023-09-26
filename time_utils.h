// Generic standard output object
//
// Copyright HOLM, 2023

#pragma once

#include <string>

#ifndef WIN32
#include <sys/prctl.h>
#endif

#include <boost/atomic.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>

// ====================================================================================
namespace chaos
{
    // ================================================================================
    static inline boost::uint64_t get_point_in_time(boost::uint32_t aux = 0)
    {
#ifndef WIN32
        boost::uint64_t rax, rdx;
        asm volatile("rdtscp\n" : "=a" (rax), "=d" (rdx), "=c" (aux) : : );
        return (rdx << 32) + rax;
#endif
        return 0;
    }

    // ================================================================================
    // Using Gregorian second clock
    // 
    // Example formats ...
    // %Y-%m-%d %f ... returns date and time in microseconds
    // %Y%m%d_%H%M%S ... returns seconds
    //
    static std::string time_as_string_greg(char const* format = "%Y-%m-%d %f")
    {
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
        boost::gregorian::date_facet* df1 = new boost::gregorian::date_facet(format);
        std::ostringstream stream;
        stream.imbue(std::locale(stream.getloc(), df1));
        stream << now;
        return stream.str();
    }

    // ================================================================================
    // Using Posix second clock
    // 
    // Example formats ...
    // %Y-%m-%d %f ... returns date and time in microseconds
    // %Y%m%d_%H%M%S ... returns seconds
    //
    static std::string time_as_string_posix(char const* format = "%Y-%m-%d %H:%M:%S")
    {
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
        boost::posix_time::time_facet* df1 = new boost::posix_time::time_facet(format);
        std::ostringstream stream;
        stream.imbue(std::locale(stream.getloc(), df1));
        stream << now;
        return stream.str();
    }

    // ================================================================================
    static std::string time_as_string(char const* format = "%Y-%m-%d %f", bool use_posix = true)
    {
        if (use_posix)
            return time_as_string_posix(format);
        else
            return time_as_string_greg(format);
    }

    // ================================================================================
    static std::string time_in_micros(const boost::posix_time::ptime& now1)
    {
        boost::posix_time::time_facet* df1 = new boost::posix_time::time_facet("%H:%M:%S.%f");
        std::ostringstream stream;
        stream.imbue(std::locale(stream.getloc(), df1));
        stream << now1;
        return stream.str();
    }

}
