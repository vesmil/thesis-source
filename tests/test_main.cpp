#include <gtest/gtest.h>

#include <stdexcept>

#include "common.h"

TEST(Ready, Mountpoint_set) {
    EXPECT_NE(TestConfig::inst().mountpoint.to_string(), "");
}

class CustomTestListener : public testing::EmptyTestEventListener {
public:
    bool ready_failed_ = false;

    void OnTestSuiteEnd(const testing::TestSuite& test_suite) override {
        if (std::string(test_suite.name()).find("Ready") != std::string::npos && test_suite.Failed()) {
            ready_failed_ = true;
        }
    }

    void OnTestStart(const testing::TestInfo& test_info) override {
        if (ready_failed_) {
            GTEST_SKIP() << "Skipping test since no mountpoint was provided.";
        }
    }
};

int main(int argc, char** argv) {
    // Parse mountpoint from command line arguments
    for (int i = 0; i < argc; i++) {
        if (std::string(argv[i]) == "--mountpoint" || std::string(argv[i]) == "-m") {
            if (i + 1 < argc) {
                TestConfig::inst().mountpoint = Path(argv[i + 1]);
                argc -= 2;

                for (int j = i; j < argc; j++) {
                    argv[j] = argv[j + 2];
                }
                break;

            } else {
                throw std::runtime_error("mountpoint not found");
            }
        }
    }

    testing::InitGoogleTest(&argc, argv);

    testing::TestEventListeners& listeners = testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new CustomTestListener);

    int test_results = RUN_ALL_TESTS();

    return test_results;
}
