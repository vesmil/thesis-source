#include "encryption_vfs.h"

#include <sodium.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "common/config.h"
#include "common/logging.h"
#include "common/prefix_parser.h"
#include "custom_vfs.h"
#include "encryptor.h"

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
    if (hook_file[0] != '#') return false;

    auto dashPos = hook_file.find('-');
    if (dashPos == std::string::npos) {
        return false;
    }

    std::string command = hook_file.substr(1, dashPos - 1);
    std::string file = hook_file.substr(dashPos + 1);

    Path parent = Path(path).parent();
    std::string file_path = parent / file;

    // TODO what if it's a directory?

    // TODO remove pass suffix - always look into directory

    if (command == "unlockPass") {
        decrypt_file(file_path, content);

        return true;
    } else if (command == "lockPass") {
        encrypt_file(file_path, content);
        return true;
    }
    if (command == "unlockKey") {
        // ...
        return true;
    } else if (command == "lockKey") {
        // ...
        return true;
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
    bool success = true;

    for (const std::string &file : get_wrapped().get_related_files(filename)) {
        Logging::Debug("Encrypting file %s", file.c_str());

        std::string fs_file_path = get_wrapped().get_fs_path(file);
        std::string output_filename = PrefixParser::apply_prefix(fs_file_path, prefix);

        // This needs to access the file system - prob use backing TODO
        std::ifstream input(fs_file_path, std::ios::binary);
        std::ofstream output(output_filename, std::ios::binary);

        if (!input.is_open()) {
            continue;
        }

        if (!output.is_open()) {
            continue;
        }

        Encryptor encryptor(password);

        success &= encryptor.encrypt_stream(input, output);

        input.close();
        output.close();

        CustomVfs::unlink(file);
    }

    return success;
}

bool EncryptionVfs::decrypt_file(const std::string &filename, const std::string &password) {
    Logging::Debug("Decrypting file %s", filename.c_str());
    std::string result_fs_path = get_wrapped().get_fs_path(filename);
    std::string input_filename = PrefixParser::apply_prefix(filename, prefix);

    // TODO this also needs access to core file
    std::ifstream input(input_filename, std::ios::binary);
    std::ofstream output(result_fs_path, std::ios::binary);

    if (!input.is_open() || !output.is_open()) {
        return false;
    }

    Encryptor encryptor(password);

    bool success = encryptor.decrypt_stream(input, output);

    input.close();
    output.close();

    CustomVfs::unlink(input_filename);
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

    // TODO mark directory as encrypted
}

void EncryptionVfs::decrypt_directory_names(const std::string &directory, const std::string &password) {
    for (const auto &file : get_wrapped().subfiles(directory)) {
        decrypt_filename(file, password);
    }
}

void EncryptionVfs::encrypt_filename(const std::string &filename, const std::string &password) {
    std::string encrypted_filename;

    Encryptor encryptor(password);
    encryptor.encrypt_string(filename, encrypted_filename);

    PrefixParser::apply_prefix(filename, prefix, {"encname"});
    get_wrapped().rename(filename, encrypted_filename, 0);
}

void EncryptionVfs::decrypt_filename(const std::string &filename, const std::string &password) {
    if (PrefixParser::args_from_prefix(filename, prefix)[0] != "encname") {
        return;
    }

    std::string decrypted_filename = PrefixParser::get_nonprefixed(filename);

    Encryptor encryptor(password);
    encryptor.decrypt_string(decrypted_filename, decrypted_filename);

    get_wrapped().rename(filename, decrypted_filename, 0);
}