#include <iostream>
#include <vector>
#include <string>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <atomic>
#include <csignal>
#include <thread>
#include <chrono>
#include "SnmpClient.hpp"
#include "SnmpResponseToOtlpConverter.hpp"
#include "HttpOtelClient.hpp"
#include "Mapping.hpp"

std::atomic<bool> running{true};
void handle_signal(int) { running = false; }



int main(int argc, char* argv[]) {

    std::string oidFile = argv[1];
    std::string mappingFile = argv[2];
    std::vector<std::string> oids = loadOidList(oidFile);
    std::cout << "LOADED OIDS" << std::endl;
    for (std::string oid : oids)
    {
        std::cout << oid << std::endl;
    }
    std::cout << "_____________________________________________" << std::endl;
    auto mappings = mappingFile.empty() ? std::unordered_map<std::string, OidMetricMapping>() 
                                       : loadMapping(mappingFile);

    std::cout << "MAPPINGS: " << std::endl;
    for (auto mapping : mappings)
    {
        std::cout << mapping.first << " : " << mapping.second.name << std::endl;
    }

    std::cout << "____________________________________________" << std::endl;



    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    std::string target = "10.0.1.138";
    std::string community = "public";
    //std::vector<std::string> oids = {"1.3.6.1.2.1.1.1.0", "1.3.6.1.2.1.1.5.0", "1.3.6.1.2.1.1.3.0", "1.3.6.1.4.1.2021.11.9.0"};
    int interval = 5;

    SnmpClient client(target, community);
    SnmpResponseToOtlpConverter converter;
    HttpOtelClient otelClient("http://172.26.16.1:4318/v1/metrics");

    int counter = 0;
    while (running) { 

        std::cout << "\nLoop #" << counter << "\n";
        netsnmp_pdu * responsePdu = client.snmpGet(oids);
        std::string otlpJson = converter.toOtlpJson(responsePdu, target, mappings);
        bool success = otelClient.sendMetrics(otlpJson);
        if (success) {
            std::cout << "Metrics sent successfully.\n";
        } else {
            std::cout << "Failed to send metrics.\n";
        }


        if (running)
            std::this_thread::sleep_for(std::chrono::seconds(1));
        counter++;

        std::cout << "____________________________________________" << std::endl;
    }

    std::cout << "Main loop exited cleanly.\n";
    return 0;
}