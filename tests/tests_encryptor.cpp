#include <gtest/gtest.h>

#include "encryptor.h"

TEST(Encryptor, encrypt_string) {
    std::string password = "test_password";
    std::string input = "Hello World!\n";

    Encryptor encryptor(password);

    std::string output;
    EXPECT_TRUE(encryptor.encrypt_string(input, output));
    EXPECT_NE(input, output);

    std::string output2;
    EXPECT_TRUE(encryptor.decrypt_string(output, output2));
    EXPECT_EQ(input, output2);
}

TEST(Encryptor, encrypt_stream) {
    std::string password = "test_password";
    std::string input = "Hello World!\n";

    Encryptor encryptor(password);

    std::stringstream input_stream(input);
    std::stringstream output_stream;
    EXPECT_TRUE(encryptor.encrypt_stream(input_stream, output_stream));

    std::string output = output_stream.str();
    EXPECT_NE(input, output);

    std::stringstream input_stream2(output);
    std::stringstream output_stream2;
    EXPECT_TRUE(encryptor.decrypt_stream(input_stream2, output_stream2));

    std::string output2 = output_stream2.str();
    EXPECT_EQ(input, output2);
}