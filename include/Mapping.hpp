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
    if (!in) throw std::runtime_error("Cannot open file: " + filename);

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
    if (!fileStream) return {};

    json json;
    fileStream >> json;

    std::unordered_map<std::string, OidMetricMapping> map;
    for (auto it = json.begin(); it != json.end(); ++it) {
        OidMetricMapping m;
        m.oid  = it.key();
        m.name = it.value().value("name", DEFAULT_METRIC_NAME_PREFIX + m.oid); 
        m.unit = it.value().value("unit", "");
        map[it.key()] = m;
    }
    return map;
}

