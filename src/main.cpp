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
#include "ArgumentParser.hpp"

std::atomic<bool> running{true};
void handle_signal(int) { running = false; }



int main(int argc, char* argv[]) {

    ArgumentParser argParser;
    argParser.parse(argc, argv);

    std::string target = argParser.target;
    std::string community = argParser.community;
    std::string oidFile = argParser.oidFile;
    std::string mappingFile = argParser.mappingFile;
    std::string otelEndpoint = argParser.otelEndpoint;
    int port = argParser.port;
    int timeout = argParser.timeout;
    int interval = argParser.interval;
    int retries = argParser.retries;
    bool verbose = argParser.verbose;


    //Parse files
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

    SnmpClient client(target, community, port);
    SnmpResponseToOtlpConverter converter;
    HttpOtelClient otelClient(otelEndpoint);

    int counter = 0;
    while (running) { 

        std::cout << "\nLoop #" << counter << "\n";
        netsnmp_pdu * responsePdu = client.snmpGet(oids);
        std::string otlpJson = converter.toOtlpJson(responsePdu, target, mappings);

        int timeoutRetryCounter = 1;
        bool success = false;
        do { 
            success = otelClient.sendMetrics(otlpJson, timeout);
        
            if (success) {
                std::cout << "Metrics sent successfully.\n";
                timeoutRetryCounter = 1;
            } else {
                std::cout << "Failed to send metrics. Retry #" << timeoutRetryCounter << std::endl << std::endl;
                timeoutRetryCounter++;
            }
        }
        while(!success && timeoutRetryCounter <= retries && running);


        if (running)
            std::this_thread::sleep_for(std::chrono::seconds(interval));
        counter++;

        std::cout << "____________________________________________" << std::endl;
    }

    std::cout << "Main loop exited cleanly.\n";
    return 0;
}