#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include "../include/json.hpp"
#include "Config.hpp"

using json = nlohmann::json;

struct OidMetricMapping
{
    std::string oid;
    std::string name;
    std::string unit;
};

inline std::vector<std::string> loadOidList(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("Cannot open OID file: " + filename);

    std::vector<std::string> oids;
    std::string line;
    while (std::getline(in, line)) {
        //trim
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty() || line[0] == '#') continue; // skip comments
        oids.push_back(line);
    }
    return oids;
}


inline std::unordered_map<std::string, OidMetricMapping> loadMapping(const std::string& filename) {
    std::ifstream fileStream(filename);
    if (!fileStream) {
        throw std::runtime_error("Cannot open mapping file: " + filename);
    }

    json j;
    try {
        fileStream >> j;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Invalid JSON in mapping file: ") + e.what());
    }

    if (!j.is_object()) {
        throw std::runtime_error("Mapping file root must be a JSON object");
    }

    std::unordered_map<std::string, OidMetricMapping> map;
    for (auto it = j.begin(); it != j.end(); ++it) {
        if (!it.value().is_object()) {
            throw std::runtime_error("Mapping for OID " + it.key() + " must be an object");
        }
        OidMetricMapping m;
        m.oid  = it.key();
        m.name = it.value().value("name", DEFAULT_METRIC_NAME_PREFIX + m.oid);
        m.unit = it.value().value("unit", "");

        if (it.value().contains("type")) {
            std::string t = it.value().value("type", "");
            if (t != "gauge") {
                throw std::runtime_error("Unsupported metric type for OID " + it.key() + ": " + t);
            }
        }

        map[it.key()] = m;
    }
    return map;
}

