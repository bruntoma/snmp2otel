#include "HttpOtelClient.hpp"
#include <curl/curl.h>
#include <iostream>
#include "Config.hpp"


HttpOtelClient::HttpOtelClient(const std::string& endpointUrl)
    : endpointUrl(endpointUrl) {}

//make curl write elsewhere
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
   (void)buffer;
   (void)userp; // shuts compiler warnings
   return size * nmemb;
}

bool HttpOtelClient::sendMetrics(const std::string& otlpJson, int timeout) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        logError("Failed to initialize curl\n");
        return false;
    }
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_URL, endpointUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, otlpJson.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, otlpJson.size());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);
    
    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OPERATION_TIMEOUTED)
    {
        logError("Timeout when exporting metrics to OTEL endpoint.");
    }

    if (res != CURLE_OK) {
        logError("curl easy perform error");
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return false;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return true;
}