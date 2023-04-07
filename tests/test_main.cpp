#include <gtest/gtest.h>

#include "custom_vfs.h"
#include "test_config.h"

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);

    CustomVfs fuseWrapper;
    TestConfig::fuseWrapper = &fuseWrapper;

    char* fuse_argv[] = {const_cast<char*>("./CustomVFS"), const_cast<char*>(TestConfig::mount_point.c_str())};
    fuseWrapper.main(2, fuse_argv);

    auto test_results = RUN_ALL_TESTS();

    std::string command = "fusermount -u " + TestConfig::mount_point;
    int unmount_result = system(command.c_str());

    if (unmount_result != 0) {
        std::cerr << "Failed to unmount fuse filesystem" << std::endl;
        return 1;
    }

    return test_results;
}