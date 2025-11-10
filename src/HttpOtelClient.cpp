#include "HttpOtelClient.hpp"
#include <curl/curl.h>
#include <iostream>
HttpOtelClient::HttpOtelClient(const std::string& endpointUrl)
    : endpointUrl(endpointUrl) {}

bool HttpOtelClient::sendMetrics(const std::string& otlpJson) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl\n";
        return false;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, endpointUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, otlpJson.c_str());
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl easy perform error" << std::endl;
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return false;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return true;
}