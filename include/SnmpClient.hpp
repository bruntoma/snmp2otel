#pragma once
#include <string>
#include <vector>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "Config.hpp"

class SnmpClient {
public:
    SnmpClient(const std::string& target, const std::string& community, int port, int timeout);
    netsnmp_pdu* snmpGet(const std::vector<std::string>& oids);

    ~SnmpClient() {
    if (session.peername != nullptr) {
        free(session.peername); 
        session.peername = nullptr;
    }
}
private:
    std::string target;
    std::string community;
    netsnmp_session session;
    int port;
    int timeout;
};