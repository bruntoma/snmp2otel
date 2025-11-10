#pragma once

#include <string>
#include <vector>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <curl/curl.h>

class HttpOtelClient {
public:
    HttpOtelClient(const std::string& endpointUrl);
    bool sendMetrics(const std::string& otlpJson);
private:
    std::string endpointUrl;
};