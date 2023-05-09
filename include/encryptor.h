#ifndef SRC_ENCRYPTOR_H
#define SRC_ENCRYPTOR_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "sodium.h"

class Encryptor {
public:
    static Encryptor from_password(const std::string &password);
    static Encryptor from_filepath(const std::string &filePath);
    static Encryptor random();

    bool encrypt_stream(std::istream &input, std::ostream &output);
    bool decrypt_stream(std::istream &input, std::ostream &output);

    void generate_file(const std::string &filePath);

private:
    Encryptor(const std::string &str, bool isPassword);
    Encryptor();

    void init_password(const std::string &password);

    void init_file(const std::string &filePath);

    unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES]{};
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES]{};
};

#endif  // SRC_ENCRYPTOR_H