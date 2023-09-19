// Generic functions
//
// Copyright HOLM, 2023

#pragma once

#include <string>
#include <cstring>
#include <cstdlib>

#ifndef WIN32
#include <sys/prctl.h>
#include <unistd.h>
#endif

// =============================================================================================
namespace chaos
{
    // =========================================================================================
    static int get_pid()
    {
#ifndef WIN32
        return getpid();
#endif
        return 0;
    }

    // =========================================================================================
    static void sleep(int value)
    {
#ifndef WIN32
        sleep(value);
#endif
    }

    // =========================================================================================
    static std::string get_environment_string(const std::string& variable)
    {
        char* env = std::getenv(variable.c_str());
        return (env == NULL) ? "" : std::string(env);
    }

    // =========================================================================================
    static std::uint32_t get_environment_int(const std::string& variable)
    {
        char* env = std::getenv(variable.c_str());
        return (env == NULL) ? 0 : atoi(env);
    }

    // =========================================================================================
    static void set_thread_name(const std::string& name)
    {
#ifndef WIN32
        prctl(PR_SET_NAME, name.c_str());
#endif
    }

    // =========================================================================================
    static void get_thread_name(std::string& name)
    {
#ifndef WIN32
        char buffer[104];
        prctl(PR_GET_NAME, buffer);
        name = buffer;
#endif
    }

    // =========================================================================================
    static std::string pin_thread_to_core(std::int32_t core)
    {
#ifndef WIN32
        // Lets set the thread affinity and lock this thread to the core configured
        cpu_set_t set;

        CPU_ZERO(&set);
        CPU_SET(core, &set);

        if (sched_setaffinity(0, sizeof(set), &set))
            return "sched_setaffinity() failed";
        else
            return "sched_setaffinity() successful";
#endif
        return "pin_thread_to_core not supported";
    }

    // =========================================================================================
    static std::string set_thread_priority()
    {
#ifndef WIN32
        // Lets set the thread priority to SCHED_FIFO so we will not be preempted
        static struct sched_param sched_param;
        memset(&sched_param, 0, sizeof(sched_param));
        sched_param.sched_priority = 90;

        if (-1 == sched_setscheduler(0, SCHED_FIFO, &sched_param))
            return "Setting the SCHED_FIFO priority failed";
        else
            return "Setting the SCHED_FIFO priority successful";
#endif
        return "set_thread_priority function not supported";
    }
}   // End of namespace
