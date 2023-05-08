#ifndef TESTS_COMMON_H
#define TESTS_COMMON_H

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "custom_vfs.h"
#include "encryption_vfs.h"
#include "versioning_vfs.h"

class TestConfig {
public:
    static TestConfig& inst() {
        static TestConfig instance;
        return instance;
    }

    CustomVfs* vfs;
    Path mountpoint;

    TestConfig(const TestConfig&) = delete;
    TestConfig& operator=(const TestConfig&) = delete;

private:
    TestConfig() {
        vfs = nullptr;
        mountpoint = Path("");
    }
};

namespace Common {

inline void clean_mountpoint() {
    for (const auto& entry : std::filesystem::directory_iterator(TestConfig::inst().mountpoint.to_string())) {
        try {
            EXPECT_TRUE(std::filesystem::remove_all(entry.path())) << "Failed to remove " << entry.path();
        } catch (const std::filesystem::filesystem_error& e) {
            FAIL() << e.what();
        }
    }
}

inline std::string read_file(const std::string& path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

inline bool write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);

    if (!file.is_open()) {
        return false;
    }

    file << content;
    file.close();

    return true;
}

}  // namespace Common

#endif