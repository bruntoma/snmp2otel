#pragma once

#include "Test.hpp"
#include "SnmpClient.hpp"

class TestSnmpClient : public Test {
public:
    bool Run() override {
        // snmpGet return null for unreachable/invalid target (for some reason this takes long, probably something to do with dns)
        {
            SnmpClient c("255.255.255.255", "public", 161, 500, 2);
            std::vector<std::string> oids = {"1.3.6.1.2.1.1.3.0"};
            netsnmp_pdu* resp = c.snmpGet(oids);
            assert_true(resp == nullptr, "snmpGet returns nullptr for unreachable/invalid target");
        }

        // snmpGet does not cause crach for invalid OIDs (skips invalid)
        {
            SnmpClient c("127.0.0.1", "public", 161, 500, 2);
            std::vector<std::string> oids = {"not-an-oid", "1.3.6.1.2.1.1.3.0"};

            try {
                c.snmpGet(oids);
            } catch (...) {
                std::cerr << "FAIL: snmpGet threw an exception for mixed oids" << std::endl;
                ++fails;
            }

            assert_true(true, "snmpGet handles mixed invalid/valid OIDs without throwing");
        }

        return fails == 0;
    }
};
