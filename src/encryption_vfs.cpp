#include "encryption_vfs.h"

#include <sodium.h>

#include <iostream>
#include <stack>

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

    try {
        if (handle_hook(pathname, content)) {
            Logging::Debug("Hook handled for %s", pathname.c_str());
            return 0;
        }
    } catch (const std::exception &e) {
        Logging::Error("Exception on handle hook for %s - %s", pathname.c_str(), e.what());
        return -1;
    }

    return get_wrapped().write(pathname, buf, count, offset, fi);
}

bool EncryptionVfs::handle_hook(const std::string &path, const std::string &content) {
    if (!PrefixParser::contains_prefix(Path::string_basename(path), prefix)) {
        return false;
    }

    auto non_prefixed = PrefixParser::remove_specific_prefix(path, prefix);
    auto args = PrefixParser::args_from_prefix(path, prefix);

    bool is_dir = get_wrapped().is_directory(path);

    if (args.size() == 1) {
        if (args[0] == "lock") {
            Encryptor encryptor{content};

            if (is_dir) {
                encrypt_directory(non_prefixed, encryptor, false);
            } else {
                return encrypt_file(non_prefixed, encryptor, true, false);
            }
        } else if (args[0] == "unlock") {
            Encryptor encryptor{content};

            if (is_dir) {
                decrypt_directory(non_prefixed, encryptor, false);
            } else {
                return decrypt_file(non_prefixed, encryptor, true, false);
            }
        } else if (args[0] == "generate") {
            Encryptor encryptor{};
            auto file_stream = CustomVfs::get_ofstream(non_prefixed, std::ios::binary);
            encryptor.generate_file(*file_stream);
            file_stream->close();

            return true;
        }

        return false;
    } else if (args.size() == 2) {
        if (args[0] == "lock") {
            std::string key_path = args[1];
            std::replace(key_path.begin(), key_path.end(), '|', '/');

            auto encryptor = Encryptor::from_file(key_path);

            if (is_dir) {
                encrypt_directory(non_prefixed, encryptor, true);
            } else {
                return encrypt_file(non_prefixed, encryptor, true, true);
            }
        } else if (args[0] == "unlock") {
            std::string key_path = args[1];
            std::replace(key_path.begin(), key_path.end(), '|', '/');

            auto encryptor = Encryptor::from_file(key_path);

            if (is_dir) {
                decrypt_directory(non_prefixed, encryptor, true);
            } else {
                return decrypt_file(non_prefixed, encryptor, true, true);
            }
        } else if (args[0] == "setKeyPath") {
            std::string key_path = args[1];
            std::replace(key_path.begin(), key_path.end(), '|', '/');

            CustomVfs::get_ofstream(key_path, std::ios::binary)
                ->write(args[1].c_str(), static_cast<int>(args[1].size()));
        }
    }

    return false;
}

int EncryptionVfs::open(const std::string &pathname, struct fuse_file_info *fi) {
    if (is_encrypted(pathname)) {
        auto args = PrefixParser::args_from_prefix(pathname, prefix);
        // TODO
    }

    return get_wrapped().open(pathname, fi);
}

int EncryptionVfs::release(const std::string &pathname, struct fuse_file_info *fi) {
    return get_wrapped().release(pathname, fi);
}

bool EncryptionVfs::is_encrypted(const std::string &pathname) const {
    return get_wrapped().exists(PrefixParser::apply_prefix(pathname, prefix));
}

std::vector<std::string> EncryptionVfs::prepare_files(const std::string &filename, bool with_related) {
    std::vector<std::string> files{};

    if (with_related) {
        files = get_wrapped().get_related_files(filename);
    } else {
        files.push_back(filename);
    }

    return files;
}

std::pair<std::unique_ptr<std::ifstream>, std::unique_ptr<std::ofstream>> EncryptionVfs::open_files(
    const std::string &input_file, const std::string &output_file, std::ios_base::openmode input_mode,
    std::ios_base::openmode output_mode) {
    auto input = CustomVfs::get_ifstream(input_file, input_mode);
    auto output = CustomVfs::get_ofstream(output_file, output_mode);

    if (!input->is_open()) {
        Logging::Error("Failed to open file %s", input_file.c_str());
    }

    if (!output->is_open()) {
        Logging::Error("Failed to open file %s", output_file.c_str());
    }

    return std::make_pair(std::move(input), std::move(output));
}

bool EncryptionVfs::encrypt_file(const std::string &filename, Encryptor &encryptor, bool with_related, bool using_key) {
    if (is_encrypted(filename)) {
        Logging::Error("File %s is already encrypted", filename.c_str());
        return false;
    }

    bool success = true;
    std::vector<std::string> encrypt_files = prepare_files(filename, with_related);

    for (const std::string &file : encrypt_files) {
        Logging::Debug("Encrypting file %s", file.c_str());

        auto [input, output] = open_files(file, PrefixParser::apply_prefix(file, prefix, {using_key ? "key" : "pass"}),
                                          std::ios::binary | std::ios::in, std::ios::binary | std::ios::out);

        bool file_success = encryptor.encrypt_stream(*input, *output);
        success = success && file_success;

        if (!success) {
            Logging::Error("Failed to encrypt file %s", file.c_str());
            continue;
        }

        input->close();
        output->close();

        auto original = CustomVfs::get_ofstream(file, std::ios::binary | std::ios::out);
        *original << "This file is encrypted" << std::endl;
        original->close();
    }

    return success;
}

bool EncryptionVfs::decrypt_file(const std::string &filename, Encryptor &encryptor, bool with_related, bool using_key) {
    bool success = true;
    std::vector<std::string> encrypt_files = prepare_files(filename, with_related);

    for (const std::string &file : encrypt_files) {
        if (PrefixParser::contains_prefix(file, prefix)) {
            continue;
        }

        Logging::Debug("Decrypting file %s", file.c_str());

        std::string input_file = PrefixParser::apply_prefix(file, prefix, {using_key ? "key" : "pass"});
        auto [input, output] = open_files(input_file, file, std::ios::binary, std::ios::binary);

        bool file_success = encryptor.decrypt_stream(*input, *output);
        success = success && file_success;

        if (!success) {
            Logging::Error("Failed to decrypt file %s", file.c_str());

            auto original = CustomVfs::get_ofstream(file, std::ios::binary | std::ios::out);
            *original << "This file is encrypted" << std::endl;
            original->close();

            continue;
        }

        input->close();
        output->close();

        CustomVfs::unlink(input_file);
    }

    return success;
}

void EncryptionVfs::encrypt_directory(const std::string &root_directory, Encryptor &encryptor, bool using_key) {
    std::stack<std::string> directories;
    directories.push(root_directory);

    while (!directories.empty()) {
        std::string current_directory = directories.top();
        directories.pop();

        for (const auto &file : CustomVfs::subfiles(current_directory)) {
            if (PrefixParser::contains_prefix(file, prefix)) {
                continue;
            }

            std::string full_path = Path(current_directory) / file;
            if (get_wrapped().is_directory(full_path)) {
                directories.push(full_path);
            } else {
                encrypt_file(full_path, encryptor, false, using_key);
            }
        }

        CustomVfs::mknod(PrefixParser::apply_prefix(current_directory, prefix), S_IFDIR | 0755, 0);
    }
}

void EncryptionVfs::decrypt_directory(const std::string &root_directory, Encryptor &encryptor, bool using_key) {
    std::stack<std::string> directories;
    directories.push(root_directory);

    while (!directories.empty()) {
        std::string current_directory = directories.top();
        directories.pop();

        for (const auto &file : CustomVfs::subfiles(current_directory)) {
            if (PrefixParser::contains_prefix(file, prefix)) {
                continue;
            }

            std::string full_path = Path(current_directory) / file;
            if (get_wrapped().is_directory(full_path)) {
                directories.push(full_path);
            } else {
                decrypt_file(full_path, encryptor, false, using_key);
            }
        }

        CustomVfs::unlink(PrefixParser::apply_prefix(current_directory, prefix));
    }
}
