#include "SnmpResponseToOtlpConverter.hpp"

std::string SnmpResponseToOtlpConverter::toOtlpJson(netsnmp_pdu* responsePdu) {
    //real implementation

    return "{" + std::string((char*)responsePdu->agent_addr) + "}";
}