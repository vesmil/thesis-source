#include "encryption_vfs.h"

#include <sodium.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "common/config.h"
#include "custom_vfs.h"
#include "encryptor.h"
#include "logging.h"

EncryptionVfs::EncryptionVfs(CustomVfs &wrapped_vfs) : VfsDecorator(wrapped_vfs) {
    if (sodium_init() == -1) {
        throw std::runtime_error("Sodium failed to initialize");
    }
}

int EncryptionVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    return get_wrapped().read(pathname, buf, count, offset, fi);
}

int EncryptionVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    if (handle_hook(pathname, buf, fi)) {
        Logging::Debug("Hook handled for %s", pathname.c_str());
        return 0;
    }

    return get_wrapped().write(pathname, buf, count, offset, fi);
}

int EncryptionVfs::readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi,
                           FuseWrapper::readdir_flags flags) {
    return get_wrapped().readdir(pathname, off, fi, flags);
}

bool EncryptionVfs::handle_hook(const std::string &path, const std::string &content, fuse_file_info *fi) {
    std::string hook_file = Path::string_basename(path);

    // TODO use prefixed...

    if (hook_file[0] == '#') {
        auto dashPos = hook_file.find('-');
        if (dashPos == std::string::npos) {
            return false;
        }

        std::string command = hook_file.substr(1, dashPos - 1);
        std::string file = hook_file.substr(dashPos + 1);

        Path parent = Path(path).parent();
        std::string file_path = parent / file;

        std::string real_file_path = get_wrapped().get_fs_path(file_path);

        // TODO what if it's a directory?

        if (command == "unlockPass") {
            // TODO for each related file, decrypt it
            decrypt_file(real_file_path, content);

            return true;
        } else if (command == "lockPass") {
            // TODO for each related file, encrypt it
            encrypt_file(real_file_path, content);
            return true;
        }
        if (command == "unlockKey") {
            // ...
            return true;
        } else if (command == "lockKey") {
            // ...
            return true;
        }
    }

    return false;
}

int EncryptionVfs::open(const std::string &pathname, struct fuse_file_info *fi) {
    std::vector<std::string> related_files = get_wrapped().get_related_files(pathname);
    for (const auto &related_file : related_files) {
        std::string realRelatedPath = get_wrapped().get_fs_path(related_file);

        // TODO check if encrypted...

        // std::string encryptedPath = get_wrapped().get_fs_path(related_file + ".enc");
    }

    return get_wrapped().open(pathname, fi);
}

int EncryptionVfs::release(const std::string &pathname, struct fuse_file_info *fi) {
    return get_wrapped().release(pathname, fi);
}

void EncryptionVfs::derive_key_and_nonce(const std::string &password, unsigned char *key, unsigned char *nonce) {
    const unsigned char salt[crypto_pwhash_SALTBYTES] = "some_fixed_salt";  // You can use a random salt
    if (crypto_pwhash(key, crypto_aead_xchacha20poly1305_ietf_KEYBYTES, password.c_str(), password.length(), salt,
                      crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE,
                      crypto_pwhash_ALG_DEFAULT) != 0) {
        Logging::Fatal("Key derivation failed.");
        return;
    }

    memcpy(nonce, key, crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
}

bool EncryptionVfs::encrypt_file(const std::string &filename, const std::string &password) {
    std::string output_filename = filename + ".enc";

    std::ifstream input(filename, std::ios::binary);
    std::ofstream output(output_filename, std::ios::binary);

    if (!input.is_open() || !output.is_open()) {
        return false;
    }

    Encryptor encryptor(password);

    bool success = encryptor.encrypt_stream(input, output);

    input.close();
    output.close();

    return success;
}

bool EncryptionVfs::decrypt_file(const std::string &filename, const std::string &password) {
    std::string output_filename = filename + ".dec";

    std::ifstream input(filename, std::ios::binary);
    std::ofstream output(output_filename, std::ios::binary);

    if (!input.is_open() || !output.is_open()) {
        return false;
    }

    Encryptor encryptor(password);

    bool success = encryptor.decrypt_stream(input, output);

    input.close();
    output.close();

    return success;
}

void EncryptionVfs::encrypt_directory(const std::string &directory, const std::string &password) {
    for (const auto &file : get_wrapped().subfiles(directory)) {
        if (get_wrapped().is_directory(file)) {
            encrypt_directory(file, password);
        } else {
            encrypt_filename(file, password);
            encrypt_file(file, password);
        }
    }
}

void EncryptionVfs::decrypt_directory_names(const std::string &directory, const std::string &password) {
    for (const auto &file : get_wrapped().subfiles(directory)) {
        decrypt_filename(file, password);
    }
}

void EncryptionVfs::encrypt_filename(const std::string &filename, const std::string &password) {}

void EncryptionVfs::decrypt_filename(const std::string &filename, const std::string &password) {}