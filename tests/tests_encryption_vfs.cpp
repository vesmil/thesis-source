#include <gtest/gtest.h>

#include <filesystem>

#include "common.h"
#include "hook-generation/encryption.h"

TEST(EncryptionVfs, password_file_lock) {
    Common::clean_mountpoint();

    std::string test_folder = Path(TestConfig::inst().mountpoint) / "enc_folder";
    std::filesystem::create_directory(test_folder);

    std::string filepath = Path(test_folder) / "test.txt";

    std::string content = "Hello World!\n";
    Common::write_file(filepath, content);

    std::string file_content = Common::read_file(filepath);
    EXPECT_EQ(file_content, content);

    std::string pass = "test";
    std::string passfile = Encryption::lock_pass_hook(filepath);
    Common::write_file(passfile, pass);

    file_content = Common::read_file(filepath);

    EXPECT_NE(file_content, "Hello World!\n");

    passfile = Encryption::unlock_pass_hook(filepath);
    Common::write_file(passfile, pass);

    file_content = Common::read_file(filepath);
    EXPECT_EQ(file_content, content);
}

TEST(EncryptionVfs, password_folder_lock) {
    Common::clean_mountpoint();

    std::string test_folder = Path(TestConfig::inst().mountpoint) / "enc_folder_2";
    std::filesystem::create_directory(test_folder);

    std::string filepath = Path(test_folder) / "test.txt";

    std::string content = "Hello World!\n";
    Common::write_file(filepath, content);

    std::string filepath2 = Path(test_folder) / "test2.txt";

    std::string content2 = "Hello World! 2\n";
    Common::write_file(filepath2, content2);

    std::string pass = "test";
    std::string passfile = Encryption::lock_pass_hook(filepath);
    Common::write_file(passfile, pass);

    std::string file_content = Common::read_file(filepath);
    EXPECT_NE(file_content, "Hello World!\n");

    passfile = Encryption::unlock_pass_hook(filepath);
    Common::write_file(passfile, pass);

    file_content = Common::read_file(filepath);
    EXPECT_EQ(file_content, content);
}

// TODO with key
