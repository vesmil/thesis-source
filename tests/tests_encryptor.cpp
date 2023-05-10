#include <gtest/gtest.h>

#include "encryptor.h"

TEST(Encryptor, password_encryptor) {
    std::string password = "test_password";
    std::string input = "Hello World!\n";

    Encryptor encryptor{password};

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

TEST(Encryptor, invalid_password_encryptor) {
    std::string password = "test_password";
    std::string input = "sdajsdakjshxcz";

    Encryptor encryptor{password};

    std::stringstream input_stream(input);
    std::stringstream output_stream;

    EXPECT_FALSE(encryptor.decrypt_stream(input_stream, output_stream));
}

TEST(Encryptor, random_encryptor) {
    std::string input = "Hello World!\n";

    Encryptor encryptor{};

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

TEST(Encryptor, invalid_random_encryptor) {
    std::string input = "sdajsdakjshxcz";

    Encryptor encryptor{};

    std::stringstream input_stream(input);
    std::stringstream output_stream;

    EXPECT_FALSE(encryptor.decrypt_stream(input_stream, output_stream));
}

TEST(Encryptor, from_file) {
    Encryptor encryptor;

    std::ofstream file("test_file");
    encryptor.generate_file(file);
    file.close();

    std::ifstream file2("test_file");
    auto encryptor2 = Encryptor(file2);
    file2.close();

    std::string input = "Hello World!\n";

    std::stringstream input_stream(input);
    std::stringstream output_stream;
    EXPECT_TRUE(encryptor2.encrypt_stream(input_stream, output_stream));

    std::string output = output_stream.str();
    EXPECT_NE(input, output);

    std::stringstream input_stream2(output);
    std::stringstream output_stream2;
    EXPECT_TRUE(encryptor2.decrypt_stream(input_stream2, output_stream2));

    std::string output2 = output_stream2.str();
    EXPECT_EQ(input, output2);
}
