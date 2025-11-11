#pragma once

#define DEFAULT_METRIC_NAME_PREFIX "snmp_"

inline bool g_verbose = false;


inline void log(const std::string& msg) {
    if (g_verbose)
        std::cout << msg << std::endl;
}

inline void logError(const std::string& msg) {
    if (g_verbose)
        std::cerr <<"ERROR: " << msg << std::endl;
}
