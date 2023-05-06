#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "custom_vfs.h"
#include "encryption_vfs.h"
#include "test_config.h"
#include "versioning_vfs.h"

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

int main(int argc, char** argv) {
    // Parse command line arguments
    bool cleanup = true;
    if (argc >= 2 && argv[1] == std::string("--no-cleanup")) {
        cleanup = false;

        argv[1] = argv[0];
        argv++;
        argc--;
    }

    std::filesystem::path mountpoint = std::filesystem::current_path() / "tests";

    // Create mountpoint
    TestConfig::inst().mountpoint = mountpoint.string();

    if (cleanup) {
        try {
            execute_system_command("fusermount -u " + TestConfig::inst().mountpoint, false);
            std::filesystem::remove_all(mountpoint);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Cleaning up failed: " << e.what() << std::endl;
            std::cerr << "Running tests anyway...";
        }
    }

    std::filesystem::create_directory(mountpoint);

    CustomVfs customVfs(TestConfig::inst().mountpoint);
    VersioningVfs versioned(customVfs);
    EncryptionVfs encrypted(versioned);
    
    TestConfig::inst().vfs = &encrypted;

    // Mount filesystem
    char* fuse_argv[] = {argv[0], const_cast<char*>(TestConfig::inst().mountpoint.c_str())};
    auto fuse_thread = std::thread(&CustomVfs::main, &encrypted, 2, fuse_argv);

    testing::InitGoogleTest(&argc, argv);
    int test_results = RUN_ALL_TESTS();

    // Unmount filesystem and clean up
    if (cleanup) {
        try {
            execute_system_command("fusermount -u " + TestConfig::inst().mountpoint);
            std::filesystem::remove_all(mountpoint);
        } catch (const std::exception& e) {
            std::cerr << "Error during cleanup: " << e.what() << std::endl;
        }
    }

    return test_results;
}
