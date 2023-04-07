#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "custom_vfs.h"
#include "test_config.h"

void execute_system_command(const std::string& command, bool check_result = true) {
    int result;

    if (check_result) {
        result = std::system(command.c_str());
    } else {
        result = std::system((command + " 1>/dev/null 2>/dev/null").c_str());
    }

    if (result != 0) {
        if (check_result) {
            throw std::runtime_error("Failed to execute system command: " + command);
        }
    }
}

void cleanup_and_prepare_mountpoint(const std::string& mountpoint) {
    try {
        execute_system_command("fusermount -u " + mountpoint, false);
        std::filesystem::remove_all(mountpoint);
        std::filesystem::create_directory(mountpoint);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Cleaning up failed: " << e.what() << std::endl;
        std::cerr << "Running tests anyway...";
    }
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);

    // Get current directory and append /tests
    std::filesystem::path mountpoint = std::filesystem::current_path() / "tests";
    TestConfig::inst().mountpoint = mountpoint.string();

    cleanup_and_prepare_mountpoint(TestConfig::inst().mountpoint);

    CustomVfs fuseWrapper;
    TestConfig::inst().vfs = &fuseWrapper;

    char* fuse_argv[] = {argv[0], const_cast<char*>(TestConfig::inst().mountpoint.c_str())};

    auto fuse_thread = std::thread(&CustomVfs::main, &fuseWrapper, 2, fuse_argv);

    int test_results = RUN_ALL_TESTS();

    try {
        execute_system_command("fusermount -u " + TestConfig::inst().mountpoint);
        std::filesystem::remove_all(mountpoint);
    } catch (const std::exception& e) {
        std::cerr << "Error during cleanup: " << e.what() << std::endl;
    }

    return test_results;
}
