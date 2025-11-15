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
    try 
    {
        ArgumentParser argParser;
        if (!argParser.parse(argc, argv)) {
            return static_cast<int>(ExitCode::INVALID_ARGS);
        }

        g_verbose = argParser.verbose;

        // parse files
        std::vector<std::string> oids;
        try {
            oids = loadOidList(argParser.oidFile);
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
        if (!argParser.mappingFile.empty()) {
            try {
                mappings = loadMapping(argParser.mappingFile);
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

        SnmpClient client(argParser.target, argParser.community, argParser.port, argParser.timeout, argParser.retries);
        SnmpResponseToOtlpConverter converter;
        HttpOtelClient otelClient(argParser.otelEndpoint);

        int counter = 0;
        while (running) { 
            log("\nLoop #" + std::to_string(counter));
            log("============================================");
            // SNMP GET
            log("--- Fetching SNMP data from " + argParser.target + " ...");
            netsnmp_pdu * responsePdu = client.snmpGet(oids);
            if (responsePdu && running) 
            {
                // convert to OTLP
                log("\n--- Converting SNMP response to OTLP JSON ...");
                std::string otlpJson = converter.toOtlpJson(responsePdu, argParser.target, mappings);
                snmp_free_pdu(responsePdu);

                // send to OTEL
                log("\n--- Sending metrics to OTEL endpoint " + argParser.otelEndpoint + " ...");
                bool success = otelClient.sendMetrics(otlpJson, argParser.timeout);
                
                if (success) 
                {
                    log("Metrics sent successfully.");
                } else {
                    logError("Failed to send metrics.");
                }
            }
            

            // sleep between scans
            if (running) 
                std::this_thread::sleep_for(std::chrono::seconds(argParser.interval));

            log("____________________________________________");
            counter++;
        }

        log("Main loop exited cleanly.");
 
    } catch (const std::exception& e) {
        logError(e.what());
        return static_cast<int>(ExitCode::UNKNOWN);
    }
   return 0;
}