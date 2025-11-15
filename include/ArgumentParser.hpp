#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <stdexcept>
#include "Config.hpp"


class ArgumentParser {
public:

    ArgumentParser() {
        target = "10.0.1.138";
        community = "public";
        oidFile = "oids.txt";
        mappingFile = "";
        otelEndpoint = "http://172.26.16.1:4318/v1/metrics";
        port = 161;
        timeout = 1000; 
        interval = 10;  
        retries = 3; 
        verbose = false;
    }

    bool parse( int argc, char** argv) {
        for (int i = 1; i < argc; i++) {
            if (std::string(argv[i]) == "-v") {
                g_verbose = true;
                verbose = true;
            }
        }

        const char* const short_options = "+t:C:o:m:e:i:r:T:p:v";
        optind = 1; 
        opterr = 0;
        
        int opt;
        while ((opt = getopt(argc, argv, short_options)) != -1) {
            try {
                switch (opt) {
                    case 't': target = optarg; break;
                    case 'C': community = optarg; break;
                    case 'o': oidFile = optarg; break;
                    case 'm': mappingFile = optarg; break;
                    case 'e': otelEndpoint = optarg; break;
                    case 'i':
                        interval = std::stoi(optarg);
                        if (interval <= 0) throw std::runtime_error("must be > 0");
                        break;
                    case 'r':
                        retries = std::stoi(optarg);
                        if (retries < 0) throw std::runtime_error("must be > 0");
                        break;
                    case 'T': 
                        timeout = std::stoi(optarg);
                        if (timeout <= 0) throw std::runtime_error("must be > 0");
                        break;
                    case 'p':
                        port = std::stoi(optarg);
                        if (port <= 0 || port > 65535) throw std::runtime_error("invalid port");
                        break;
                    case 'v': 
                        verbose = true; 
                        break;
                    case '?':
                        throw std::runtime_error(optopt ? std::string("Unknown or invalid option -") + (char)optopt : "Unknown argument");
                }
            } catch (const std::exception& e) {
                logError(std::string("Invalid argument -") + (optopt ? (char)optopt : (char)opt) + ". " + e.what());            
                printUsage();
                return false;
            }
        }
        
        //required params
        if (target.empty()) {
            logError("missing -t (target).");
            printUsage();
            return false;
        }
        if (oidFile.empty()) {
            logError("mising -o (oids_file).");
            printUsage();
            return false;
        }

        if (otelEndpoint.empty()) {
            logError("missing -e (OTEL endpoint).");
            printUsage();
            return false;
        }
        return true;
    }

    void printUsage() const {
        log("Usage:");
        log("  snmp2otel -t target [-C community] -o oids_file -e endpoint [-i interval] [-r retries] [-T timeout] [-p port] [-v]");
        log("  -t target — IP or DNS name of SNMP agent.");
        log("  -C community — SNMP v2c community string. Default: public.");
        log("  -o oids_file — file with list of OIDs to query.");
        log("  -e endpoint — OTEL endpoint URL (OTLP/HTTP JSON).");
        log("  -i interval — polling period in seconds (>0). Default: 10.");
        log("  -r retries — retransmissions on timeout. Default: 2.");
        log("  -T timeout — SNMP timeout in ms. Default: 1000.");
        log("  -p port — UDP port. Default: 161.");
        log("  -v — verbose mode.");
    }

    std::string target;
    std::string community;
    std::string oidFile;
    std::string mappingFile;
    std::string otelEndpoint;
    int port;
    int timeout;
    int interval;
    int retries;
    bool verbose;
};