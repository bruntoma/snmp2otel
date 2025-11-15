#include "SnmpClient.hpp"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

SnmpClient::SnmpClient(const std::string& target, const std::string& community, int port, int timeout, int retries) : target(target), community(community), port(port), timeout(timeout), retries(retries) {
    init_snmp("snmpapp");
    snmp_sess_init(&session);
    session.peername = strdup((target + ":" + std::to_string(port)).c_str());
    session.version = SNMP_VERSION_2c;
    session.community = (u_char*)strdup(community.c_str());
    session.community_len = (int)community.length();
    session.timeout = timeout;
    session.retries = retries;

    snmp_set_save_descriptions(0);
    snmp_set_mib_warnings(0);
    netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_NUMERIC_OIDS, 1);
}

netsnmp_pdu* SnmpClient::snmpGet(const std::vector<std::string>& oids) {
    netsnmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GET);
    for (const auto& oidStr : oids) {
        oid anOID[MAX_OID_LEN];
        size_t anOID_len = MAX_OID_LEN;
        if (!snmp_parse_oid(oidStr.c_str(), anOID, &anOID_len)) {
            logError(std::string("Invalid OID skipped: ") + oidStr);
            continue;
        }
        snmp_add_null_var(pdu, anOID, anOID_len);
    }

    netsnmp_pdu* responsePdu = nullptr;
    netsnmp_session* ss = snmp_open(&session);
    if (!ss) {
        logError(std::string("Failed to open SNMP session to ") + target);
        return nullptr;
    }

    int stat = snmp_synch_response(ss, pdu, &responsePdu);
    if (stat != STAT_SUCCESS || !responsePdu) {
        logError("SNMP GET request failed (no response).");
        snmp_close(ss);
        return nullptr;
    }

    snmp_close(ss);
    return responsePdu;
}