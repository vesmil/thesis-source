#include <gtest/gtest.h>

#include "custom_vfs.h"
#include "test_config.h"

TEST(CustomVfs, test_dir) {
    CustomVfs* fuseWrapper = TestConfig::inst().vfs;

    fuseWrapper->mkdir("/test", 0777);
    fuseWrapper->mkdir("/test/dir", 0777);

    std::vector<std::string> files = fuseWrapper->subfiles("/test");
    std::vector<std::string> expected = {"/test/dir"};

    EXPECT_EQ(files, expected);
}

TEST(CustomVfs, test_file) {
    CustomVfs* fuseWrapper = TestConfig::inst().vfs;

    fuseWrapper->mkdir("/test", 0777);
    fuseWrapper->mkdir("/test/dir", 0777);

    fuseWrapper->mknod("/test/dir/helloworld.txt", 0666, 0);

    std::vector<std::string> files = fuseWrapper->subfiles("/test/dir");
    std::vector<std::string> expected = {"/test/dir/helloworld.txt"};

    EXPECT_EQ(files, expected);
}

TEST(CustomVfs, test_file_content) {
    CustomVfs* fuseWrapper = TestConfig::inst().vfs;

    fuseWrapper->mkdir("/test", 0777);
    fuseWrapper->mkdir("/test/dir", 0777);

    fuseWrapper->mknod("/test/dir/helloworld.txt", 0666, 0);

    std::string content = "Hello, world.\n";

    fuseWrapper->open("/test/dir/helloworld.txt", nullptr);
    fuseWrapper->write("/test/dir/helloworld.txt", content.c_str(), content.size(), 0, nullptr);

    char buffer[content.size()];
    fuseWrapper->read("/test/dir/helloworld.txt", buffer, content.size(), 0, nullptr);
    std::string result(buffer, content.size());

    EXPECT_EQ(result, content);
}