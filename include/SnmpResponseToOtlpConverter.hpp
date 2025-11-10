#pragma once

#include <string>
#include <vector>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

class SnmpResponseToOtlpConverter {
public:
    std::string toOtlpJson(netsnmp_pdu* responsePdu
    );
};