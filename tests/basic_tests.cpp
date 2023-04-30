#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "custom_vfs.h"
#include "test_config.h"

TEST(CustomVfs, test_dir) {
    for (const auto& entry : std::filesystem::directory_iterator(TestConfig::inst().mountpoint)) {
        std::filesystem::remove_all(entry.path());
    }

    std::filesystem::create_directory(TestConfig::inst().mountpoint + "/test/");
    std::filesystem::create_directory(TestConfig::inst().mountpoint + "/test/dir");

    std::vector<std::string> files;

    for (const auto& entry : std::filesystem::directory_iterator(TestConfig::inst().mountpoint + "/test")) {
        files.push_back(entry.path());
    }

    std::vector<std::string> expected = {TestConfig::inst().mountpoint + "/test/dir"};

    EXPECT_EQ(files, expected);
}

TEST(CustomVfs, test_file) {
    for (const auto& entry : std::filesystem::directory_iterator(TestConfig::inst().mountpoint)) {
        std::filesystem::remove_all(entry.path());
    }

    std::filesystem::create_directory(TestConfig::inst().mountpoint + "/test/");
    std::ofstream file(TestConfig::inst().mountpoint + "/test/file");
    file << "test";
    file.close();

    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::directory_iterator(TestConfig::inst().mountpoint + "/test")) {
        files.push_back(entry.path());
    }

    std::vector<std::string> expected = {TestConfig::inst().mountpoint + "/test/file"};
    EXPECT_EQ(files, expected);

    std::ifstream file2(TestConfig::inst().mountpoint + "/test/file");
    std::string content;
    file2 >> content;
    file2.close();
    EXPECT_EQ(content, "test");
}