#pragma once

#define DEFAULT_METRIC_NAME_PREFIX "snmp_"

#include <iostream>

inline bool g_verbose = false;

enum class ExitCode {
    OK = 0,
    INVALID_ARGS = 1,
    MISSING_OID_FILE = 2,
    INVALID_OID_FILE = 3,
    INVALID_MAPPING_FILE = 4,
    OTEL_ERR = 5,
    SNMP_ERR = 6,
    UNKNOWN = 7
};

inline void log(const std::string& msg) {
    if (g_verbose)
        std::cout << msg << std::endl;
}


inline void logError(const std::string& msg) {
    if (g_verbose)
        std::cerr << "ERROR: " << msg << std::endl;
}
