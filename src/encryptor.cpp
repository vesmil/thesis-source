#ifndef __aarch64__

#include "encryptor.h"

#include <sodium.h>

#include <stdexcept>
#include <vector>

#include "common/logging.h"

Encryptor Encryptor::from_password(const std::string &password) {
    return {password, true};
}

Encryptor Encryptor::from_filepath(const std::string &filePath) {
    return {filePath, false};
}

Encryptor Encryptor::random() {
    return {};
}

Encryptor::Encryptor(const std::string &str, bool isPassword) {
    if (sodium_init() == -1) {
        throw std::runtime_error("Sodium failed to initialize");
    }

    if (isPassword) {
        init_password(str);
    } else {
        init_file(str);
    }
}

Encryptor::Encryptor() {
    if (sodium_init() == -1) {
        throw std::runtime_error("Sodium failed to initialize");
    }

    randombytes_buf(key, crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    randombytes_buf(nonce, crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
}

void Encryptor::init_password(const std::string &password) {
    const unsigned char salt[crypto_pwhash_SALTBYTES] = "fixed_salt";

    if (crypto_pwhash(key, crypto_aead_xchacha20poly1305_ietf_KEYBYTES, password.c_str(), password.length(), salt,
                      crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_DEFAULT) != 0) {
        throw std::runtime_error("Key derivation failed.");
    }

    memcpy(nonce, key, crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
}

void Encryptor::init_file(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for reading.");
    }

    // Read key
    file.read(reinterpret_cast<char *>(key), crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    if (!file) {
        throw std::runtime_error("Failed to read key from file.");
    }

    // Read nonce
    file.read(reinterpret_cast<char *>(nonce), crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    if (!file) {
        throw std::runtime_error("Failed to read nonce from file.");
    }

    file.close();
}

void Encryptor::generate_file(const std::string &filePath) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        Logging::Fatal("Failed to open file for writing.");
    }

    file.write(reinterpret_cast<const char *>(key), crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
    if (!file) {
        throw std::runtime_error("Failed to write key to file.");
    }

    file.write(reinterpret_cast<const char *>(nonce), crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    if (!file) {
        throw std::runtime_error("Failed to write nonce to file.");
    }

    file.close();
}

bool Encryptor::encrypt_stream(std::istream &input, std::ostream &output) {
    std::vector<unsigned char> buf(std::istreambuf_iterator<char>(input), {});

    std::vector<unsigned char> encrypted(buf.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long encrypted_len;
    crypto_aead_xchacha20poly1305_ietf_encrypt(encrypted.data(), &encrypted_len, buf.data(), buf.size(), nullptr, 0,
                                               nullptr, nonce, key);

    output.write(reinterpret_cast<char *>(encrypted.data()), static_cast<int>(encrypted_len));

    return true;
}

bool Encryptor::decrypt_stream(std::istream &input, std::ostream &output) {
    std::vector<unsigned char> buf(std::istreambuf_iterator<char>(input), {});

    if (buf.size() < crypto_aead_xchacha20poly1305_ietf_ABYTES) {
        return false;
    }

    std::vector<unsigned char> decrypted(buf.size() - crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long decrypted_len;

    if (crypto_aead_xchacha20poly1305_ietf_decrypt(decrypted.data(), &decrypted_len, nullptr, buf.data(), buf.size(),
                                                   nullptr, 0, nonce, key) != 0) {
        return false;
    }

    output.write(reinterpret_cast<char *>(decrypted.data()), static_cast<int>(decrypted_len));

    return true;
}

#endif