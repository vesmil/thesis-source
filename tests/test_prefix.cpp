#include <gtest/gtest.h>

#include "prefix_parser.h"

TEST(Prefix, parse_simple) {
    std::string path = "/test/something/dir";
    std::string prefix = "TEST";
    std::vector<std::string> args = {"aaa", "123", "xxx"};

    std::string result = PrefixParser::apply_prefix(path, prefix, args);

    EXPECT_EQ(result, "/test/something/#TEST-aaa-123-xxx#dir");

    auto resultVector = PrefixParser::args_from_prefix(result, prefix);

    EXPECT_EQ(resultVector, args);
}

TEST(Prefix, parse_multiple) {
    std::string path = "/test/something/dir";
    std::string prefix = "TEST";
    std::vector<std::string> args = {"aaa", "123", "xxx"};

    std::string prefix2 = "TEST2";
    std::vector<std::string> args2 = {"bbb", "456", "yyy"};

    std::string result = PrefixParser::apply_prefix(path, prefix, args);
    result = PrefixParser::apply_prefix(result, prefix2, args2);

    EXPECT_EQ(result, "/test/something/#TEST2-bbb-456-yyy##TEST-aaa-123-xxx#dir");

    auto resultVector = PrefixParser::args_from_prefix(result, prefix);
    auto resultVector2 = PrefixParser::args_from_prefix(result, prefix2);

    EXPECT_EQ(resultVector, args);
    EXPECT_EQ(resultVector2, args2);
}

TEST(Prefix, empty_arg) {
    std::string path = "/test/something/dir";
    std::string prefix = "TEST";

    std::string result = PrefixParser::apply_prefix(path, prefix, {});

    EXPECT_EQ(result, "/test/something/#TEST#dir");

    auto resultVector = PrefixParser::args_from_prefix(result, prefix);
    EXPECT_EQ(resultVector, std::vector<std::string>());
}

TEST(Prefix, basename) {
    std::string path = "/test/something/#TEST-aaa-123-xxx##xxxx-#dir";

    std::string result = PrefixParser::get_nonprefixed(path);
    EXPECT_EQ(result, "/test/something/dir");

    std::string path2 = "/test/something/#TEST-aaa-123-xxx##xxxx-#di#r";

    std::string result2 = PrefixParser::get_nonprefixed(path2);
    EXPECT_EQ(result2, "/test/something/di#r");
}