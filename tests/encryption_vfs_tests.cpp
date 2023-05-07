#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "common.h"

TEST(CustomVfs, pass_lock) {
    Common::clean_mountpoint();

    std::string test_folder = Path(TestConfig::inst().mountpoint) / "test_folder";
    std::filesystem::create_directory(test_folder);

    std::string filepath = Path(test_folder) / "test.txt";
    
    std::string content = "Hello World!";
    Common::write_file(filepath, content);

    std::string file_content = Common::read_file(filepath);
    EXPECT_EQ(file_content, content);

    std::string pass = "test";
    std::string passfile = Path(test_folder) / "#lock-test.txt";
    Common::write_file(passfile, pass);

    file_content = Common::read_file(filepath);
    EXPECT_NE(file_content, content);

    passfile = Path(test_folder) / "#unlock-test.txt";
    Common::write_file(passfile, pass);

    file_content = Common::read_file(filepath);
    EXPECT_EQ(file_content, content);
}