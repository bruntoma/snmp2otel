#pragma once

#include "Test.hpp"
#include "SnmpClient.hpp"

class TestSnmpClient : public Test {
public:
    bool Run() override {
        // snmpGet return null for unreachable/invalid target (for some reason this takes long, probably something to do with dns)
        {
            SnmpClient c("256.256.256.256", "public", 161, 50000);
            std::vector<std::string> oids = {"1.3.6.1.2.1.1.3.0"};
            netsnmp_pdu* resp = c.snmpGet(oids);
            // We expect nullptr because target is invalid/unreachable
            assert_true(resp == nullptr, "snmpGet returns nullptr for unreachable/invalid target");
        }

        // snmpGet does not cause crach for invalid OIDs (skips invalid)
        {
            SnmpClient c("127.0.0.1", "public", 161, 50000);
            std::vector<std::string> oids = {"not-an-oid", "1.3.6.1.2.1.1.3.0"};
            netsnmp_pdu* resp = nullptr;
            try {
                resp = c.snmpGet(oids);
            } catch (...) {
                std::cerr << "FAIL: snmpGet threw an exception for mixed oids" << std::endl;
                ++fails;
            }
            if (resp) {
                snmp_free_pdu(resp);
            }
            assert_true(true, "snmpGet handles mixed invalid/valid OIDs without throwing");
        }

        return fails == 0;
    }
};
