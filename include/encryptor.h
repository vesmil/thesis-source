#ifndef SRC_ENCRYPTOR_H
#define SRC_ENCRYPTOR_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "sodium.h"

class Encryptor {
public:
    explicit Encryptor(std::string password);

    bool encrypt_string(const std::string &input, std::string &output);
    bool decrypt_string(const std::string &input, std::string &output);

    bool encrypt_stream(std::istream &input, std::ostream &output);
    bool decrypt_stream(std::istream &input, std::ostream &output);

private:
    static void derive_key_and_nonce(const std::string &password, unsigned char *key, unsigned char *nonce);

    std::string password_;
};

#endif  // SRC_ENCRYPTOR_H