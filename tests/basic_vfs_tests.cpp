#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "common.h"
#include "custom_vfs.h"

TEST(CustomVfs, create_dir) {
    Common::clean_mountpoint();

    std::string test_folder = TestConfig::inst().mountpoint / "test";
    EXPECT_TRUE(std::filesystem::create_directory(test_folder));

    std::string nested_folder = Path(test_folder) / "dir";
    EXPECT_TRUE(std::filesystem::create_directory(nested_folder));

    std::vector<std::string> files;

    for (const auto& entry : std::filesystem::directory_iterator(test_folder)) {
        files.push_back(entry.path());
    }

    std::vector<std::string> expected = {nested_folder};

    EXPECT_EQ(files, expected);
}

TEST(CustomVfs, create_file) {
    Common::clean_mountpoint();

    std::filesystem::create_directory((TestConfig::inst().mountpoint / "test/").to_string());
    Common::write_file(TestConfig::inst().mountpoint / "/test/file", "test");

    std::vector<std::string> files;
    for (const auto& entry :
         std::filesystem::directory_iterator((TestConfig::inst().mountpoint / "/test").to_string())) {
        files.push_back(entry.path());
    }

    std::vector<std::string> expected = {TestConfig::inst().mountpoint / "/test/file"};
    EXPECT_EQ(files, expected);

    std::ifstream file2(TestConfig::inst().mountpoint / "/test/file");
    std::string content;
    file2 >> content;
    file2.close();
    EXPECT_EQ(content, "test");
}