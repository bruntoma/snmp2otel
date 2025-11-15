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
    try {
        ArgumentParser argParser;
        if (!argParser.parse(argc, argv)) {
            return static_cast<int>(ExitCode::INVALID_ARGS);
        }

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

        // Parse files
        std::vector<std::string> oids;
        try {
            oids = loadOidList(oidFile);
            log("LOADED OIDS");
        } catch (const std::exception& e) {
            logError(std::string("Failed to load OID file: ") + e.what());
            return static_cast<int>(ExitCode::MISSING_OID_FILE);
        }

        for (std::string oid : oids)
        {
            log(oid);
        }
        log("_____________________________________________");
        std::unordered_map<std::string, OidMetricMapping> mappings;
        if (!mappingFile.empty()) {
            try {
                mappings = loadMapping(mappingFile);
            } catch (const std::exception& e) {
                logError(std::string("Invalid mapping file: ") + e.what());
                return static_cast<int>(ExitCode::INVALID_MAPPING_FILE);
            }
        }

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
            if (!responsePdu) {
                logError("SNMP request failed, retry scheduled");
                // respect interval and continue
                if (running)
                    std::this_thread::sleep_for(std::chrono::seconds(interval));
                counter++;
                continue;
            }
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
 
    } catch (const std::exception& e) {
        logError(e.what());
        return static_cast<int>(ExitCode::UNKNOWN);
    }
   return 0;
}