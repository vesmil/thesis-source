#ifdef __aarch64__

#include <utility>

#include "encryptor.h"

// This is just placeholder code for the ARM64 version of the encryptor.

Encryptor::Encryptor(std::string password) : password_(std::move(password)) {}

void Encryptor::derive_key_and_nonce(const std::string &password, unsigned char *key, unsigned char *nonce) {}

bool Encryptor::encrypt_string(const std::string &input, std::string &output) {
    return true;
}

bool Encryptor::decrypt_string(const std::string &input, std::string &output) {
    return true;
}

bool Encryptor::encrypt_stream(std::istream &input, std::ostream &output) {
    return true;
}

bool Encryptor::decrypt_stream(std::istream &input, std::ostream &output) {
    return true;
}

#endif
