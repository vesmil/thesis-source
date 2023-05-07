#include <gtest/gtest.h>

#include <stdexcept>

#include "common.h"

int main(int argc, char** argv) {
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
    }

    if (TestConfig::inst().mountpoint.to_string().empty()) {
        throw std::runtime_error("No mountpoint provided");
    }

    testing::InitGoogleTest(&argc, argv);
    int test_results = RUN_ALL_TESTS();

    Common::clean_mountpoint();
    
    return test_results;
}
