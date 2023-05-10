#include <gtest/gtest.h>

#include "common/path.h"

TEST(Path, pathlib_simple) {
    Path path("/test/something/dir");
    EXPECT_EQ(Path::string_parent(path.to_string()), "/test/something");
    EXPECT_EQ(Path::string_basename(path.to_string()), "dir");

    EXPECT_EQ(path.parent().to_string(), "/test/something");
    EXPECT_EQ(path.basename().to_string(), "/dir");
}

TEST(Path, pathlib_simple2) {
    Path path("/dir/");
    EXPECT_EQ(Path::string_parent(path.to_string()), "/");
    EXPECT_EQ(Path::string_basename(path.to_string()), "dir");

    EXPECT_EQ(path.parent().to_string(), "/");
    EXPECT_EQ(path.basename().to_string(), "/dir");
}

TEST(Path, pathlib_edge) {
    Path path("/");
    EXPECT_EQ(Path::string_parent(path.to_string()), "");
    EXPECT_EQ(Path::string_basename(path.to_string()), "");

    EXPECT_EQ(path.parent().to_string(), "");
    EXPECT_EQ(path.basename().to_string(), "");
}

TEST(Path, pathlib_addition) {
    Path path("/test/something/dir");

    EXPECT_EQ((path / "/test/"), "/test/something/dir/test");
    EXPECT_EQ((path / "test"), "/test/something/dir/test");

    path = Path("/test/something/dir/");
    path /= "/test/";
    EXPECT_EQ(path, "/test/something/dir/test");
}

TEST(Path, pathlib_equal) {
    Path path("/test/something/dir");

    EXPECT_EQ(path, "/test/something/dir");
    EXPECT_EQ(path, Path("/test/something/dir"));
    EXPECT_EQ(path, Path("/test/something/dir/"));

    EXPECT_NE(path, "/test/something/dir2");
    EXPECT_NE(path, Path("/test/something/dir2"));
}
