#pragma once
#include <string>
#include <vector>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "Config.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>

class SnmpClient {
public:
    SnmpClient(const std::string& target, const std::string& community, int port, int timeout, int retries);
    netsnmp_pdu* snmpGet(const std::vector<std::string>& oids, int retryIndex = 0);

    ~SnmpClient() {
        if (session.peername != nullptr) {
            free(session.peername); 
            session.peername = nullptr;
        }

        if (session.community != nullptr) {
            free(session.community); 
            session.community = nullptr;
        }

        snmp_close_sessions();
        snmp_shutdown("snmpapp");
    }

    bool isValidIp(const std::string& address) {
         struct sockaddr_in sockaddr; 
    
         return inet_pton(AF_INET, address.c_str(), &(sockaddr.sin_addr)) != 0;
    }

private:
    std::string target;
    std::string community;
    netsnmp_session session;
    int port;
    int timeout;
    int retries;
};