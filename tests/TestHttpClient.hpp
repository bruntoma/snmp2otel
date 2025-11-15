#pragma once

#include "Test.hpp"
#include "HttpOtelClient.hpp"

class TestHttpClient : public Test {
public:
    bool Run() override {
        // bad URL should return false
        {
            HttpOtelClient c("ht!tp://bad::url");
            bool ok = c.sendMetrics("{}", 1000);
            assert_true(!ok, "sendMetrics returns false for malformed URL");
        }

        // no server should return false
        {
            HttpOtelClient c("http://127.0.0.1:9/metrics");
            bool ok = c.sendMetrics("{\"resourceMetrics\":[]}", 500);
            assert_true(!ok, "sendMetrics returns false when no server listening");
        }

        // empty endpoint should return false
        {
            HttpOtelClient c("");
            bool ok = c.sendMetrics("{}", 1000);
            assert_true(!ok, "sendMetrics returns false for empty endpoint");
        }

        return fails == 0;
    }
};
