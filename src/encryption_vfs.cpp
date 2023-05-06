#include "encryption_vfs.h"

#include <sodium.h>

#include <fstream>
#include <iostream>

#include "common/config.h"
#include "custom_vfs.h"

EncryptionVfs::EncryptionVfs(CustomVfs &wrapped_vfs) : VfsDecorator(wrapped_vfs) {
    if (sodium_init() == -1) {
        throw std::runtime_error("Sodium failed to initialize");
    }
}

int EncryptionVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    if (handle_hook(pathname, fi)) {
        return 0;
    }

    return CustomVfs::read(pathname, buf, count, offset, fi);
}

bool EncryptionVfs::handle_hook(const std::string &path, fuse_file_info *fi) {
    if (path[0] == '#') {
        // TODO

        return true;
    }

    return false;
}

int EncryptionVfs::open(const std::string &pathname, struct fuse_file_info *fi) {
    auto realPath = get_fs_path(pathname);

    return CustomVfs::open(pathname, fi);
}

int EncryptionVfs::release(const std::string &pathname, struct fuse_file_info *fi) {
    return CustomVfs::release(pathname, fi);
}

void EncryptionVfs::derive_key_and_nonce(const std::string &password, unsigned char *key, unsigned char *nonce) {
    const unsigned char salt[crypto_pwhash_SALTBYTES] = "some_fixed_salt";  // You can use a random salt
    if (crypto_pwhash(key, crypto_aead_xchacha20poly1305_ietf_KEYBYTES, password.c_str(), password.length(), salt,
                      crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_DEFAULT) != 0) {
        // TODO dont' exit
        std::cerr << "Key derivation failed." << std::endl;
        std::exit(1);
    }

    memcpy(nonce, key, crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
}

bool EncryptionVfs::encrypt_file(const std::string &input_filename, const std::string &output_filename,
                                 const std::string &password) {
    std::ifstream input(input_filename, std::ios::binary);
    std::ofstream output(output_filename, std::ios::binary);

    if (!input.is_open() || !output.is_open()) {
        return false;
    }

    // Derive key and nonce
    unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES];
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
    derive_key_and_nonce(password, key, nonce);

    // Read input file
    std::vector<unsigned char> buf(std::istreambuf_iterator<char>(input), {});
    input.close();

    // Encrypt the file
    std::vector<unsigned char> encrypted(buf.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long encrypted_len;
    crypto_aead_xchacha20poly1305_ietf_encrypt(encrypted.data(), &encrypted_len, buf.data(), buf.size(), nullptr, 0,
                                               nullptr, nonce, key);

    // Write encrypted data to output file
    output.write(reinterpret_cast<char *>(encrypted.data()), encrypted_len);
    output.close();

    return true;
}

bool EncryptionVfs::decrypt_file(const std::string &input_filename, const std::string &output_filename,
                                 const std::string &password) {
    std::ifstream input(input_filename, std::ios::binary);
    std::ofstream output(output_filename, std::ios::binary);

    if (!input.is_open() || !output.is_open()) {
        return false;
    }

    // Derive key and nonce
    unsigned char key[crypto_aead_xchacha20poly1305_ietf_KEYBYTES];
    unsigned char nonce[crypto_aead_xchacha20poly1305_ietf_NPUBBYTES];
    derive_key_and_nonce(password, key, nonce);

    // Read input file
    std::vector<unsigned char> buf(std::istreambuf_iterator<char>(input), {});
    input.close();

    // Check if the input file size is large enough to contain the authentication tag
    if (buf.size() < crypto_aead_xchacha20poly1305_ietf_ABYTES) {
        std::cerr << "Invalid ciphertext." << std::endl;
        return false;
    }

    // Decrypt the file
    std::vector<unsigned char> decrypted(buf.size() - crypto_aead_xchacha20poly1305_ietf_ABYTES);
    unsigned long long decrypted_len;
    if (crypto_aead_xchacha20poly1305_ietf_decrypt(decrypted.data(), &decrypted_len, nullptr, buf.data(), buf.size(),
                                                   nullptr, 0, nonce, key) != 0) {
        // Decryption failed
        std::cerr << "Decryption failed. Possibly due to an incorrect password or corrupted data." << std::endl;
        return false;
    }

    // Write decrypted data to output file
    output.write(reinterpret_cast<char *>(decrypted.data()), decrypted_len);
    output.close();

    return true;
}