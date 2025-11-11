#pragma once
#include <string>
#include <vector>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "Mapping.hpp"

class SnmpResponseToOtlpConverter {
public:

    std::string toOtlpJson(netsnmp_pdu* responsePdu, const std::string& agentIp, std::unordered_map<std::string, OidMetricMapping>& mappings);

};