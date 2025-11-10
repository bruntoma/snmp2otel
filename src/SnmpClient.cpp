#include "SnmpClient.hpp"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

SnmpClient::SnmpClient(const std::string& target, const std::string& community) : target(target), community(community) {
    init_snmp("snmpapp");
    snmp_sess_init(&session);
    session.peername = strdup(target.c_str());
    session.version = SNMP_VERSION_2c;
    session.community = (u_char*)strdup(community.c_str());
    session.community_len = community.length();
}

netsnmp_pdu* SnmpClient::snmpGet(const std::vector<std::string>& oids) {
    netsnmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GET);
    for (const auto& oidStr : oids) {
        oid anOID[MAX_OID_LEN];
        size_t anOID_len = MAX_OID_LEN;
        if (!snmp_parse_oid(oidStr.c_str(), anOID, &anOID_len)) {
            continue;
        }
        snmp_add_null_var(pdu, anOID, anOID_len);
    }

    netsnmp_pdu* responsePdu = nullptr;
    netsnmp_session* ss = snmp_open(&session);
    if (!ss) {
        return nullptr;
    }

    int status = snmp_synch_response(ss, pdu, &responsePdu);
    if (status != STAT_SUCCESS || !responsePdu) {
        snmp_close(ss);
        return nullptr;
    }

    snmp_close(ss);
    return responsePdu;
}