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
        const char* const short_options = "t:C:o:m:e:i:r:T:p:v";
        
        int opt;

        optind = 1; 
        opterr = 0;

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
                        if (optopt) {
                            logError("Argument -" + std::to_string((char)optopt));
                        } else {
                            logError("Unknown argument");
                        }
                        return false;
                    default: return false;
                }
            } catch (const std::exception& e) {
                logError("Invalid argument -" + std::to_string((char)opt) + ". "  + e.what());
                return false;
            }
        }
        
        //required params
        if (target.empty()) {
            logError("missing -t (target).");
            return false;
        }
        if (oidFile.empty()) {
            logError("mising -o (oids_file).");
            return false;
        }

        if (otelEndpoint.empty()) {
            logError("missing -e (OTEL endpoint).");
            return false;
        }
        return true;
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