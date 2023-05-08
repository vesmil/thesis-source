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
#ifndef __aarch64__
    if (sodium_init() == -1) {
        throw std::runtime_error("Sodium failed to initialize");
    }
#endif
}

int EncryptionVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    return get_wrapped().read(pathname, buf, count, offset, fi);
}

int EncryptionVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    std::string content(buf, count);
    if (handle_hook(pathname, content, fi)) {
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
    if (is_encrypted(pathname)) {
        // TODO check key
    }

    return get_wrapped().open(pathname, fi);
}

int EncryptionVfs::release(const std::string &pathname, struct fuse_file_info *fi) {
    return get_wrapped().release(pathname, fi);
}

bool EncryptionVfs::is_encrypted(const std::string &pathname) const {
    return get_wrapped().exists(PrefixParser::apply_prefix(pathname, prefix));
}

bool EncryptionVfs::encrypt_file(const std::string &filename, const std::string &password) {
    if (is_encrypted(filename)) {
        Logging::Error("File %s is already encrypted", filename.c_str());
        return false;
    }

    bool success = true;

    for (const std::string &file : get_wrapped().get_related_files(filename)) {
        Logging::Debug("Encrypting file %s", file.c_str());

        std::string output_file = PrefixParser::apply_prefix(file, prefix);

        auto input = CustomVfs::get_ifstream(file, std::ios::binary | std::ios::in);
        auto output = CustomVfs::get_ofstream(output_file, std::ios::binary | std::ios::out);

        if (!input->is_open()) {
            Logging::Error("Failed to open file %s", file.c_str());
            continue;
        }

        if (!output->is_open()) {
            Logging::Error("Failed to open file %s", output_file.c_str());
            continue;
        }

        Encryptor encryptor(password);
        success &= encryptor.encrypt_stream(*input, *output);

        input->close();
        output->close();

        auto original = CustomVfs::get_ofstream(file, std::ios::binary | std::ios::out);
        *original << "This file is encrypted" << std::endl;
        original->close();
    }

    return success;
}

bool EncryptionVfs::decrypt_file(const std::string &filename, const std::string &password) {
    // TODO decrypt all related

    Logging::Debug("Decrypting file %s", filename.c_str());

    std::string input_file = PrefixParser::apply_prefix(filename, prefix);

    auto input = CustomVfs::get_ifstream(input_file, std::ios::binary);
    auto output = CustomVfs::get_ofstream(filename, std::ios::binary);

    if (!input->is_open()) {
        Logging::Error("Failed to open file %s", input_file.c_str());
        return false;
    }

    if (!output->is_open()) {
        Logging::Error("Failed to open file %s", filename.c_str());
        return false;
    }

    Encryptor encryptor(password);

    bool success = encryptor.decrypt_stream(*input, *output);

    input->close();
    output->close();

    CustomVfs::unlink(input_file);
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
