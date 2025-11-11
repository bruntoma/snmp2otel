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
#include "Config.hpp"

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
    g_verbose = verbose;

    //Parse files
    std::vector<std::string> oids = loadOidList(oidFile);
    log("LOADED OIDS");

    for (std::string oid : oids)
    {
        log(oid);
    }
    log("_____________________________________________");
    auto mappings = mappingFile.empty() ? std::unordered_map<std::string, OidMetricMapping>() 
                                       : loadMapping(mappingFile);

    log("MAPPINGS: ");
    for (auto mapping : mappings)
    {
        log(mapping.first + " : " + mapping.second.name);
    }

    log("____________________________________________");



    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    SnmpClient client(target, community, port);
    SnmpResponseToOtlpConverter converter;
    HttpOtelClient otelClient(otelEndpoint);

    int counter = 0;
    while (running) { 

        log("\nLoop #" + std::to_string(counter));
        netsnmp_pdu * responsePdu = client.snmpGet(oids);
        std::string otlpJson = converter.toOtlpJson(responsePdu, target, mappings);

        int timeoutRetryCounter = 1;
        bool success = false;
        do { 
            success = otelClient.sendMetrics(otlpJson, timeout);
        
            if (success) {
                log("Metrics sent successfully.");
                timeoutRetryCounter = 1;
            } else {
                logError("Failed to send metrics. Retry #" + std::to_string(timeoutRetryCounter) + "\n");
                timeoutRetryCounter++;
            }
        }
        while(!success && timeoutRetryCounter <= retries && running);


        if (running)
            std::this_thread::sleep_for(std::chrono::seconds(interval));
        counter++;

        log("____________________________________________");
    }

    log("Main loop exited cleanly.");
    return 0;
}