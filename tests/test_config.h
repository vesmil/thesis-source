#pragma once

#include <string>

#include "custom_vfs.h"

class TestConfig {
public:
    static TestConfig& inst() {
        static TestConfig instance;
        return instance;
    }

    CustomVfs* vfs;
    std::string mountpoint;

    TestConfig(const TestConfig&) = delete;
    TestConfig& operator=(const TestConfig&) = delete;

private:
    TestConfig() {
        vfs = nullptr;
        mountpoint = "/mnt/custom_vfs";
    }
};
