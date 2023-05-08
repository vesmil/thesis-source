#include <gtest/gtest.h>

#include "common.h"

TEST(VersioningVfs, restore_version) {
    Common::clean_mountpoint();

    std::string test_folder = Path(TestConfig::inst().mountpoint) / "ver_folder";
    std::filesystem::create_directory(test_folder);

    std::string filepath = Path(test_folder) / "test.txt";

    std::string content = "Hello World!\n";
    Common::write_file(filepath, content);

    std::string file_content = Common::read_file(filepath);
    EXPECT_EQ(file_content, content);

    std::string content2 = "Hello World! 2\n";
    Common::write_file(filepath, content2);

    file_content = Common::read_file(filepath);
    EXPECT_EQ(file_content, content2);

    std::string restore_file = Path(test_folder) / "#restore_1-test.txt";
    Common::write_file(restore_file, " ");

    file_content = Common::read_file(filepath);
    EXPECT_EQ(file_content, content);
}

TEST(VersioningVfs, delete_version) {
    Common::clean_mountpoint();

    std::string test_folder = Path(TestConfig::inst().mountpoint) / "ver_folder";
    std::filesystem::create_directory(test_folder);

    std::string filepath = Path(test_folder) / "test.txt";

    std::string content = "Test\nfile";
    Common::write_file(filepath, content);

    std::string file_content = Common::read_file(filepath);
    EXPECT_EQ(file_content, content);

    std::string content2 = "New version of\tthat file";
    Common::write_file(filepath, content2);

    file_content = Common::read_file(filepath);
    EXPECT_EQ(file_content, content2);

    std::string delete_file = Path(test_folder) / "#delete_1-test.txt";
    Common::write_file(delete_file, " ");

    std::string restore_file = Path(test_folder) / "#restore_1-test.txt";
    Common::write_file(restore_file, " ");

    file_content = Common::read_file(filepath);
    EXPECT_NE(file_content, content);
}