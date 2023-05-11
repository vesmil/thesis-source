#ifndef SRC_ENCRYPTOR_H
#define SRC_ENCRYPTOR_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "sodium.h"

/// @brief Wrapper around ChaCha20-Poly1305 encryption
class Encryptor {
public:
    /// @brief Generates encryptor from password
    explicit Encryptor(const std::string &str);

    /// @brief Generates encryptor from file-stream
    explicit Encryptor(std::istream &fileStream);

    /// @brief Generates random encryptor
    Encryptor();

    /// @brief Generates encryptor from filesystem path
    static Encryptor from_file(const std::string &filePath);

    bool encrypt_stream(std::istream &input, std::ostream &output) const;
    bool decrypt_stream(std::istream &input, std::ostream &output) const;

    /// @brief Stores the Encryptor key to a file
    void store_key(std::ostream &fileStream);

private:
    void init_password(const std::string &password);
    void init_file(std::istream &filePath);

    unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES]{};
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES]{};
};

#endif  // SRC_ENCRYPTOR_H
