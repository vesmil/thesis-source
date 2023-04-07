#include <gtest/gtest.h>

#include "custom_vfs.h"
#include "test_config.h"

TEST(CustomVfs, test_dir) {
    CustomVfs* fuseWrapper = TestConfig::fuseWrapper;

    fuseWrapper->mkdir("/test", 0777);
    fuseWrapper->mkdir("/test/dir", 0777);

    std::vector<std::string> files = fuseWrapper->subfiles("/test");
    std::vector<std::string> expected = {"/test/dir"};
    EXPECT_EQ(files, expected);
}
