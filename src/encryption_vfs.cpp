#include "encryption_vfs.h"

#include <sodium.h>

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

int EncryptionVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    std::string content(buf, count);
    if (handle_hook(pathname, content, fi)) {
        Logging::Debug("Hook handled for %s", pathname.c_str());
        return 0;
    }

    return get_wrapped().write(pathname, buf, count, offset, fi);
}

bool EncryptionVfs::handle_hook(const std::string &path, const std::string &content, fuse_file_info *fi) {
    std::string hook_file = Path::string_basename(path);

    if (!PrefixParser::contains_prefix(hook_file, prefix)) {
        return false;
    }

    auto non_prefixed = PrefixParser::remove_specific_prefix(hook_file, prefix);
    auto args = PrefixParser::args_from_prefix(hook_file, prefix);

    if (args.size() == 1) {
        if (args[0] == "encrypt") {
            return encrypt(non_prefixed, content, true);
        } else if (args[0] == "decrypt") {
            return decrypt(non_prefixed, content, true);
        }
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

bool EncryptionVfs::encrypt_file(const std::string &filename, const std::string &password, bool with_related) {
    if (is_encrypted(filename)) {
        Logging::Error("File %s is already encrypted", filename.c_str());
        return false;
    }

    bool success = true;

    std::vector<std::string> encrypt_files{};

    if (with_related) {
        encrypt_files = get_wrapped().get_related_files(filename);
    } else {
        encrypt_files.push_back(filename);
    }

    for (const std::string &file : encrypt_files) {
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

        Encryptor encryptor = Encryptor::from_password(password);
        success &= encryptor.encrypt_stream(*input, *output);

        input->close();
        output->close();

        auto original = CustomVfs::get_ofstream(file, std::ios::binary | std::ios::out);
        *original << "This file is encrypted" << std::endl;
        original->close();
    }

    return success;
}

bool EncryptionVfs::decrypt_file(const std::string &filename, const std::string &password, bool with_related) {
    bool success = true;

    std::vector<std::string> encrypt_files{};

    if (with_related) {
        encrypt_files = get_wrapped().get_related_files(filename);
    } else {
        encrypt_files.push_back(filename);
    }

    for (const std::string &file : encrypt_files) {
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

        Encryptor encryptor = Encryptor::from_password(password);

        success = encryptor.decrypt_stream(*input, *output);

        input->close();
        output->close();

        CustomVfs::unlink(input_file);
    }

    return success;
}

void EncryptionVfs::encrypt_directory(const std::string &directory, const std::string &password) {
    for (const auto &file : CustomVfs::subfiles(directory)) {
        if (PrefixParser::contains_prefix(file, prefix)) {
            continue;
        }

        if (get_wrapped().is_directory(file)) {
            encrypt_directory(file, password);
        } else {
            encrypt_file(Path(directory) / file, password, false);
        }
    }

    // TODO encrypt directory name - mby recursive?
}

void EncryptionVfs::decrypt_directory(const std::string &directory, const std::string &password) {
    for (const auto &file : CustomVfs::subfiles(directory)) {
        if (PrefixParser::contains_prefix(file, prefix)) {
            continue;
        }

        if (get_wrapped().is_directory(file)) {
            decrypt_directory(file, password);
        } else {
            decrypt_file(Path(directory) / file, password, false);
        }
    }
}