#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

#include "ArgumentParser.hpp"
#include "Mapping.hpp"
#include "Test.hpp"
#include "Config.hpp"

class TestFilesParsing : public Test {
public:
    bool Run() override
    {
        // valid oid file
        std::string tmp_oid_valid = "tests_tmp_oids_valid.txt";
        {
            std::ofstream f(tmp_oid_valid);
            f << "# comment\n";
            f << "1.3.6.1.2.1.1.3.0\n";
            f << "1.3.6.1.2.1.1.5.0\n";
            f.close();

            try {
                auto v = loadOidList(tmp_oid_valid);
                assert_true(v.size() == 2, "valid oid file - two oids read");
            } catch (const std::exception& e) {
                std::cerr << "FAIL: loadOidList threw for valid file: " << e.what() << std::endl;
                ++fails;
            }
        }

        // nonexistent oid file
        {
            assert_throws([&](){ loadOidList("nonexistent_oids.txt"); }, "nonexistent oid file");
        }

        // valid mapping file
        std::string tmp_map_valid = "test_mappings.json";
        {
            std::ofstream f(tmp_map_valid);
            f << "{\n";
            f << "  \"1.3.6.1.2.1.1.3.0\": { \"name\": \"snmp.sysUpTime\", \"unit\": \"ms\", \"type\": \"gauge\" }\n";
            f << "}\n";
            f.close();

            try {
                auto m = loadMapping(tmp_map_valid);
                assert_true(m.size() == 1 && m.count("1.3.6.1.2.1.1.3.0"), "valid mapping file parsed");
            } catch (const std::exception& e) {
                std::cerr << "FAIL: loadMapping threw for valid mapping: " << e.what() << std::endl;
                ++fails;
            }
        }

        // invalid mapping file (contains nonsense)
        std::string tmp_map_invalid = "tests_tmp_map_invalid.json";
        {
            std::ofstream f(tmp_map_invalid);
            f << "not a json";
            f.close();
            assert_throws([&](){ loadMapping(tmp_map_invalid); }, "invalid mapping JSON should throw");
        }

        // mapping file with missing name field - should generate default name
        std::string tmp_map_missing_name = "tests_mappings_missing_name.json";
        {
            std::ofstream f(tmp_map_missing_name);
            f << "{\n";
            f << "  \"1.3.6.1.2.1.1.3.0\": { \"unit\": \"ms\", \"type\": \"gauge\" }\n";
            f << "}\n";
            f.close();

            try {
                auto m = loadMapping(tmp_map_missing_name);
                auto it = m.find("1.3.6.1.2.1.1.3.0");
                assert_true(it != m.end() && it->second.name.find(DEFAULT_METRIC_NAME_PREFIX) == 0, "mapping missing name -> use name with default prefix");
            } catch (const std::exception& e) {
                std::cerr << "FAIL: loadMapping threw for mapping missing name: " << e.what() << std::endl;
                ++fails;
            }
        }

        // mapping file with non-gauge type - should throw
        std::string tmp_map_non_gauge = "tests_tmp_map_nongauge.json";
        {
            std::ofstream f(tmp_map_non_gauge);
            f << "{\n";
            f << "  \"1.3.6.1.2.1.1.3.0\": { \"name\": \"m\", \"type\": \"counter\" }\n";
            f << "}\n";
            f.close();

            assert_throws([&](){ loadMapping(tmp_map_non_gauge); }, "mapping with non-gauge type should throw");
        }

        // cleanup
        std::remove(tmp_oid_valid.c_str());
        std::remove(tmp_map_valid.c_str());
        std::remove(tmp_map_invalid.c_str());
        std::remove(tmp_map_missing_name.c_str());
        std::remove(tmp_map_non_gauge.c_str());

        return fails == 0;
    }
};