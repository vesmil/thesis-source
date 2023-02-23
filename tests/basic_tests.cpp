#include <gtest/gtest.h>
#include "Node.h"

TEST(NodeTest, Example) {
Node node("example", false, nullptr);
EXPECT_EQ(node.getName(), "example");
}
