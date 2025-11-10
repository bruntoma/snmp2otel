#pragma once
#include <string>
#include <vector>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

class SnmpClient {
public:
    SnmpClient(const std::string& target, const std::string& community);
    netsnmp_pdu* snmpGet(const std::vector<std::string>& oids);
private:
    std::string target;
    std::string community;
    netsnmp_session session;
};