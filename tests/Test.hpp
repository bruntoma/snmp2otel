#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

#include "ArgumentParser.hpp"
#include "Mapping.hpp"

class Test { 
public:
    int fails = 0;

    void assert_true(bool cond, const std::string &msg) {
        if (!cond) {
            std::cerr << "FAIL: " << msg << std::endl;
            ++fails;
        } else {
            std::cerr << "OK: " << msg << std::endl;
        }
    }

    void assert_throws(std::function<void()> fn, const std::string &msg) {
        try {
            fn();
            std::cerr << "FAIL: expected exception: " << msg << std::endl;
            fails++;
        } catch (...) {
            std::cerr << "OK: threw as expected: " << msg << std::endl;
        }
    }

    virtual bool Run() = 0;
};