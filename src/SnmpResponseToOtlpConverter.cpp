#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <cctype>
#include "../include/json.hpp"

#include "SnmpResponseToOtlpConverter.hpp"

using json = nlohmann::json;

static std::string snmp_type_to_string(u_char type) {
    switch(type) {
        case ASN_INTEGER:    return "INTEGER";
        case ASN_TIMETICKS:  return "TIMETICKS";
        case ASN_OCTET_STR:  return "OCTET STR";
        case ASN_OBJECT_ID:  return "OBJECT ID";
        case ASN_SEQUENCE:   return "SEQUENCE";
        default:             return "UNKNOWN";
    }
}

struct MetricInfo {
    std::string name;
    double value;
    long long timestamp;
};

long long get_time() {
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    return duration_cast<nanoseconds>(now.time_since_epoch()).count(); // to ns
}

bool str_to_double(std::string str, double& result)
{
    try {
        result = std::stod(str);
        return true;        
    } catch (const std::invalid_argument& e) {
        std::cerr << "Conversion error '" << str << "' is not a number string" << std::endl;
        return false;
    }
}

std::vector<MetricInfo> extractSnmpData(netsnmp_pdu* responsePdu)
{
    std::vector<MetricInfo> infoList;
    long long timestamp = get_time();
    
    for (netsnmp_variable_list * vars = responsePdu->variables; vars != NULL; vars = vars->next_variable) 
    { 
        bool err = false;
        
        // oid to str
        char oid_str_buf[256]; 
        snprint_objid(oid_str_buf, sizeof(oid_str_buf), vars->name, vars->name_length);
        std::string oid_str(oid_str_buf);
        std::string metric_name = "snmp_value_" + oid_str;
        std::replace(metric_name.begin(), metric_name.end(), '.', '_');

        // process value
        double value = 0.0;
        switch (vars->type) {
            
            // snmp number to double (gauge)
            case ASN_INTEGER:
            case ASN_COUNTER:
            case ASN_GAUGE:
            case ASN_TIMETICKS:
                value = (double)*vars->val.integer;
                break;

            // if octet string contains number, export it as double (gauge). Fail otherwise
            case ASN_OCTET_STR:
            {
                std::string oct_str((char *)vars->val.string, vars->val_len);
                if (!str_to_double(oct_str, value)) // if it fails, err
                {
                    std::cerr << "OID " + oid_str + ": OCTET STRING '" + oct_str + "' does not contain number" << std::endl;
                    err = true;
                }
                break;
            }

            // other types not supported
            default:
                std::cerr << "OID " + oid_str + ": Unsupported type: " + snmp_type_to_string(vars->type) << std::endl;
                err = true;
                break;
        }

        if (!err) 
        {
            infoList.push_back({
                metric_name,
                value,
                timestamp
            });
        }
    }
    
    return infoList;
}

std::string SnmpResponseToOtlpConverter::toOtlpJson(netsnmp_pdu* responsePdu, const std::string& agentIp)
{
    std::vector<MetricInfo> metrics = extractSnmpData(responsePdu);
   
   json root_json = {
    {"resourceMetrics", json::array({
        {
            {"resource", {
                {"attributes", json::array({
                    {
                        //telemetry source
                        {"key", "service.name"},
                        {"value", {{"stringValue", "snmp2otel"}}}
                    },
                    {
                        //agent
                        {"key", "snmp.target.address"},
                        {"value", {{"stringValue", agentIp}}}
                    }
                })}
            }},
            {"scopeMetrics", json::array({
                {
                    {"scope", json::object()},
                    {"metrics", json::array()}
                }
            })}
        }
    })}
};

    json& metrics_array = root_json["resourceMetrics"][0]["scopeMetrics"][0]["metrics"]; // ref to the array with metrics
    for (const auto& info : metrics) {
        json metric_json = {
            {"name", info.name}, 
            {"gauge", {
                //todo: use mapping file
                {"dataPoints", {
                    {
                        {"timeUnixNano", std::to_string(info.timestamp)},
                        {"asDouble", info.value}
                    }
                }}
            }}
        };
        
        metrics_array.push_back(metric_json);
    }
    return root_json.dump(0);
}
