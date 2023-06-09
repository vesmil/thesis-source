#include <gtest/gtest.h>

#include <stdexcept>

#include "common.h"

int main(int argc, char** argv) {
    bool no_fuse = false;

    // Mountpoint or no-fuse has to be provided
    for (int i = 0; i < argc; i++) {
        if (std::string(argv[i]) == "--mountpoint" || std::string(argv[i]) == "-m") {
            if (i + 1 < argc) {
                TestConfig::inst().mountpoint = Path(argv[i + 1]);
                argc -= 2;

                for (int j = i; j < argc; j++) {
                    argv[j] = argv[j + 2];
                }
            }

            break;
        }

        if (std::string(argv[i]) == "--no-fuse" || std::string(argv[i]) == "-n") {
            no_fuse = true;
            argc -= 1;

            for (int j = i; j < argc; j++) {
                argv[j] = argv[j + 1];
            }

            break;
        }
    }

    if (TestConfig::inst().mountpoint.to_string().empty() && !no_fuse) {
        throw std::runtime_error("No mountpoint provided");
    }

    testing::InitGoogleTest(&argc, argv);

    // Remove tests that require fuse
    if (no_fuse) {
        testing::GTEST_FLAG(filter) = "-EncryptionVfs.*:VersioningVfs.*:CustomVfs.*";
    }

    int test_results = RUN_ALL_TESTS();

    if (!no_fuse) {
        Common::clean_mountpoint();
    }

    return test_results;
}
