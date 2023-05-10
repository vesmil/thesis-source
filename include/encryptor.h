#ifndef SRC_ENCRYPTOR_H
#define SRC_ENCRYPTOR_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "sodium.h"

class Encryptor {
public:
    explicit Encryptor(const std::string &str);
    explicit Encryptor(std::istream &fileStream);
    Encryptor();

    static Encryptor from_file(const std::string &filePath);

    bool encrypt_stream(std::istream &input, std::ostream &output);
    bool decrypt_stream(std::istream &input, std::ostream &output);

    void generate_file(std::ostream &fileStream);

private:
    void init_password(const std::string &password);

    void init_file(std::istream &filePath);

    unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES]{};
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES]{};
};

#endif  // SRC_ENCRYPTOR_H
