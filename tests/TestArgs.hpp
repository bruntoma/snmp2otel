#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

#include "ArgumentParser.hpp"
#include "Mapping.hpp"
#include "Test.hpp"

class TestArgs : public Test {
public:
    bool Run() override
    {
        // no args
        {
            const char* argv[] = {"prog"};
            ArgumentParser p;
            bool ok = p.parse(1, const_cast<char**>(argv));
            assert_true(!ok, "no arguments should fail parse");
        }

        // missing -o (oids)
        {
            const char* argv[] = {"program", "-t", "127.0.0.1", "-e", "http://localhost:4318/v1/metrics"};
            ArgumentParser p;
            bool ok = p.parse(5, const_cast<char**>(argv));
            assert_true(!ok, "missing -o should fail parse");
        }

        // missing -e (endpoint)
        {
            const char* argv[] = {"program", "-t", "127.0.0.1", "-o", "test_oids.txt"};
            ArgumentParser p;
            bool ok = p.parse(5, const_cast<char**>(argv));
            assert_true(!ok, "missing -e should fail parse");
        }

        // mapping file (optional)
        std::string tmp_mappings_file = "test_mappings.json";
        {
            std::ofstream f(tmp_mappings_file);
            f << "{\"1.3.6.1.2.1.1.3.0\": { \"name\": \"snmp.koza\", \"unit\": \"cm\", \"type\": \"gauge\" }}";
            f.close();

            const char* argv[] = {"program", "-t", "127.0.0.1", "-o", "placeholder.txt", "-m", tmp_mappings_file.c_str(), "-e", "http://localhost:4318/v1/metrics" };
            ArgumentParser p;
            bool ok = p.parse(9, const_cast<char**>(argv));
            assert_true(ok, "valid args with mapping file should parse");

            auto mapping = loadMapping(tmp_mappings_file);
            assert_true(mapping.find("1.3.6.1.2.1.1.3.0")->second.name == "snmp.koza", "loadMapping reads mapping correctly");
        }

        // valid args with OID file present
        std::string tmp_oid_file = "tests_tmp_oids.txt";
        {
            std::ofstream f(tmp_oid_file);
            f << "# comment\n";
            f << "1.3.6.1.2.1.1.3.0\n";
            f.close();

            const char* argv[] = {"program", "-t", "127.0.0.1", "-o", tmp_oid_file.c_str(), "-e", "http://localhost:4318/v1/metrics"};
            ArgumentParser p;
            bool ok = p.parse(7, const_cast<char**>(argv));
            assert_true(ok, "valid args with existing oids should parse");

            try {
                std::vector<std::string> v = loadOidList(tmp_oid_file);
                assert_true(v.size() == 1 && v[0] == "1.3.6.1.2.1.1.3.0", "loadOidList finds one oid");
            } catch (const std::exception& e) {
                std::cerr << "FAIL: loadOidList threw: " << e.what() << std::endl;
                fails++;
            }
        }

        // loadMapping throws for missing file
        assert_throws([&](){ loadMapping("nonexistent_mapping.json"); }, "loadMapping missing file");

        // loadMapping throws for invalid type
        std::string tmp_map = "tests_tmp_map.json";
        {
            std::ofstream f(tmp_map);
            f << "{\n";
            f << "  \"1.3.6.1.2.1.1.3.0\": { \"type\": \"blbytyp\" }\n";
            f << "}\n";
            f.close();

            assert_throws([&](){ loadMapping(tmp_map); }, "loadMapping rejects non-gauge type");
        }

        // numeric arguments intervals
        {
            // interval <= 0
            const char* a1[] = {"program", "-t", "127.0.0.1", "-o", "x", "-e", "u", "-i", "0"};
            ArgumentParser p1;
            bool ok1 = p1.parse(9, const_cast<char**>(a1));
            assert_true(!ok1, "interval 0 should fail parse");

            const char* a2[] = {"program", "-t", "127.0.0.1", "-o", "x", "-e", "u", "-i", "-5"};
            ArgumentParser p2;
            bool ok2 = p2.parse(9, const_cast<char**>(a2));
            assert_true(!ok2, "interval negative should fail parse");

            // retries < 0
            const char* a3[] = {"program", "-t", "127.0.0.1", "-o", "x", "-e", "u", "-r", "-1"};
            ArgumentParser p3;
            bool ok3 = p3.parse(9, const_cast<char**>(a3));
            assert_true(!ok3, "retries negative should fail parse");

            // timeout <= 0
            const char* a4[] = {"program", "-t", "127.0.0.1", "-o", "x", "-e", "u", "-T", "0"};
            ArgumentParser p4;
            bool ok4 = p4.parse(9, const_cast<char**>(a4));
            assert_true(!ok4, "timeout 0 should fail parse");

            // invalid port
            const char* a5[] = {"program", "-t", "127.0.0.1", "-o", "x", "-e", "u", "-p", "0"};
            ArgumentParser p5;
            bool ok5 = p5.parse(9, const_cast<char**>(a5));
            assert_true(!ok5, "port 0 should fail parse");

            const char* a6[] = {"program", "-t", "127.0.0.1", "-o", "x", "-e", "u", "-p", "70000"};
            ArgumentParser p6;
            bool ok6 = p6.parse(9, const_cast<char**>(a6));
            assert_true(!ok6, "port >65535 should fail parse");

            // valid numeric values
            const char* a7[] = {"program", "-t", "127.0.0.1", "-o", "x", "-e", "u", "-i", "5", "-r", "2", "-T", "1000", "-p", "161"};
            ArgumentParser p7;
            bool ok7 = p7.parse(15, const_cast<char**>(a7));
            assert_true(ok7, "valid numeric args should parse");
        }

        // cleanup
        std::remove(tmp_mappings_file.c_str());
        std::remove(tmp_oid_file.c_str());
        std::remove(tmp_map.c_str());

        if (fails) {
            std::cerr << "Tests failed: " << fails << std::endl;
            return false;
        }
        std::cerr << "All tests passed." << std::endl;
        return true;
    }
};