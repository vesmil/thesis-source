#include "encryptor.h"

#include <utility>

Encryptor::Encryptor(std::string password) : password_(std::move(password)) {}

void Encryptor::derive_key_and_nonce(const std::string &password, unsigned char *key, unsigned char *nonce) {
    const unsigned char salt[crypto_pwhash_SALTBYTES] = "fixed_salt";

    if (crypto_pwhash(key, crypto_aead_xchacha20poly1305_ietf_KEYBYTES, password.c_str(), password.length(), salt,
                      crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_DEFAULT) != 0) {
        throw std::runtime_error("Key derivation failed.");
    }

    memcpy(nonce, key, crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
}

bool Encryptor::encrypt_string(const std::string &input, std::string &output) {
    unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES];
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];

    derive_key_and_nonce(password_, key, nonce);

    std::vector<unsigned char> encrypted(input.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long encrypted_len;
    crypto_aead_xchacha20poly1305_ietf_encrypt(encrypted.data(), &encrypted_len,
                                               reinterpret_cast<const unsigned char *>(input.data()), input.size(),
                                               nullptr, 0, nullptr, nonce, key);

    output.assign(encrypted.begin(), encrypted.begin() + static_cast<int>(encrypted_len));
    return true;
}

bool Encryptor::decrypt_string(const std::string &input, std::string &output) {
    unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES];
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
    derive_key_and_nonce(password_, key, nonce);

    if (input.size() < crypto_aead_xchacha20poly1305_ietf_ABYTES) {
        return false;
    }

    std::vector<unsigned char> decrypted(input.size() - crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long decrypted_len;
    if (crypto_aead_xchacha20poly1305_ietf_decrypt(decrypted.data(), &decrypted_len, nullptr,
                                                   reinterpret_cast<const unsigned char *>(input.data()), input.size(),
                                                   nullptr, 0, nonce, key) != 0) {
        return false;
    }

    output.assign(decrypted.begin(), decrypted.begin() + static_cast<int>(decrypted_len));
    return true;
}

bool Encryptor::encrypt_stream(std::istream &input, std::ostream &output) {
    unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES];
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];

    derive_key_and_nonce(password_, key, nonce);

    std::vector<unsigned char> buf(std::istreambuf_iterator<char>(input), {});

    std::vector<unsigned char> encrypted(buf.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long encrypted_len;
    crypto_aead_xchacha20poly1305_ietf_encrypt(encrypted.data(), &encrypted_len, buf.data(), buf.size(), nullptr, 0,
                                               nullptr, nonce, key);

    output.write(reinterpret_cast<char *>(encrypted.data()), static_cast<int>(encrypted_len));

    return true;
}

bool Encryptor::decrypt_stream(std::istream &input, std::ostream &output) {
    unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES];
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
    derive_key_and_nonce(password_, key, nonce);
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
