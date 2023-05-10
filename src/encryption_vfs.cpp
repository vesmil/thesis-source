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
        if (is_hook(pathname)) {
            if (!handle_hook(pathname, content)) {
                Logging::Debug("Failed to handle hook for %s", pathname.c_str());
                return -1;
            }

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
        return handle_single_arg(non_prefixed, args[0], content, is_dir);
    } else if (args.size() == 2) {
        return handle_double_arg(non_prefixed, args[0], args[1], is_dir);
    }

    return false;
}

bool EncryptionVfs::handle_single_arg(const std::string &non_prefixed, const std::string &arg,
                                      const std::string &content, bool is_dir) {
    Encryptor encryptor;
    bool is_key = false;

    if (arg == "lock" || arg == "unlock") {
        encryptor = Encryptor{content};
    } else if (arg == "defaultLock") {
        if (!get_default_key_encryptor(encryptor)) {
            return false;
        }
        is_key = true;
    } else if (arg == "generate") {
        return generate_encryption_file(non_prefixed);
    } else {
        return false;
    }

    return handle_encryption_action(non_prefixed, arg, encryptor, is_dir, is_key);
}

bool EncryptionVfs::handle_double_arg(const std::string &non_prefixed, const std::string &arg,
                                      const std::string &key_path_arg, bool is_dir) {
    if (arg != "lock" && arg != "unlock" && arg != "setDefault") {
        return false;
    }

    if (arg == "setDefault") {
        return set_default_key(key_path_arg);
    }

    std::string key_path = key_path_arg;
    std::replace(key_path.begin(), key_path.end(), '|', '/');
    auto encryptor = Encryptor::from_file(key_path);

    return handle_encryption_action(non_prefixed, arg, encryptor, is_dir, true);
}

bool EncryptionVfs::generate_encryption_file(const std::string &non_prefixed) {
    Encryptor encryptor{};
    auto file_stream = CustomVfs::get_ofstream(non_prefixed, std::ios::binary);
    encryptor.generate_file(*file_stream);
    file_stream->close();

    return true;
}

bool EncryptionVfs::set_default_key(const std::string &key_path_arg) {
    std::string key_path = key_path_arg;
    std::replace(key_path.begin(), key_path.end(), '|', '/');

    auto path_file = CustomVfs::get_ofstream(Config::encryption.path_to_key_path, std::ios::binary);
    *path_file << key_path;
    path_file->close();

    return true;
}

bool EncryptionVfs::handle_encryption_action(const std::string &non_prefixed, const std::string &arg,
                                             const Encryptor &encryptor, bool is_dir, bool use_key_file) {
    if (arg == "lock" || arg == "defaultLock") {
        return is_dir ? encrypt_directory(non_prefixed, encryptor, use_key_file)
                      : encrypt_file(non_prefixed, encryptor, true, use_key_file);
    } else if (arg == "unlock") {
        return is_dir ? decrypt_directory(non_prefixed, encryptor, use_key_file)
                      : decrypt_file(non_prefixed, encryptor, true, use_key_file);
    }

    return false;
}

int EncryptionVfs::open(const std::string &pathname, struct fuse_file_info *fi) {
    try {
        if (is_encrypted(pathname)) {
            std::string key_locked = PrefixParser::apply_prefix(pathname, prefix, {"key"});
            Encryptor encryptor;
            if (get_wrapped().exists(key_locked) && get_default_key_encryptor(encryptor)) {
                if (decrypt_file(pathname, encryptor, true, true)) {
                    std::string temp_unlocked_indicator = PrefixParser::apply_prefix(pathname, prefix, {"tmp"});
                    get_wrapped().mknod(temp_unlocked_indicator, 0666, 0);

                    return get_wrapped().open(pathname, fi);
                }
            }
        }
    } catch (const std::exception &e) {
        Logging::Error("Exception on open for %s - %s", pathname.c_str(), e.what());
        return -1;
    }

    return get_wrapped().open(pathname, fi);
}

int EncryptionVfs::release(const std::string &pathname, struct fuse_file_info *fi) {
    std::string temp_unlocked_indicator = PrefixParser::apply_prefix(pathname, prefix, {"tmp"});
    if (get_wrapped().exists(temp_unlocked_indicator) && get_wrapped().exists(Config::encryption.path_to_key_path)) {
        get_wrapped().release(pathname, fi);
        get_wrapped().unlink(temp_unlocked_indicator);

        Encryptor encryptor;
        if (get_default_key_encryptor(encryptor)) {
            encrypt_file(pathname, encryptor, true, true);
            return 0;
        }
    }

    return get_wrapped().release(pathname, fi);
}

bool EncryptionVfs::is_encrypted(const std::string &pathname) const {
    return get_wrapped().exists(PrefixParser::apply_prefix(pathname, prefix, {"key"})) ||
           get_wrapped().exists(PrefixParser::apply_prefix(pathname, prefix, {"pass"}));
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

std::pair<std::unique_ptr<std::ifstream>, std::unique_ptr<std::ofstream>> EncryptionVfs::prepare_streams(
    const std::string &input_file, const std::string &output_file) {
    auto input = CustomVfs::get_ifstream(input_file, std::ios::binary);
    auto output = CustomVfs::get_ofstream(output_file, std::ios::binary);

    if (!input->is_open()) {
        Logging::Error("Failed to open file %s", input_file.c_str());
    }
    if (!output->is_open()) {
        Logging::Error("Failed to open file %s", output_file.c_str());
    }

    return std::make_pair(std::move(input), std::move(output));
}

bool EncryptionVfs::encrypt_file(const std::string &filename, const Encryptor &encryptor, bool with_related,
                                 bool using_key) {
    if (is_encrypted(filename)) {
        Logging::Error("File %s is already encrypted", filename.c_str());
        return false;
    }

    bool success = true;
    std::vector<std::string> encrypt_files = prepare_files(filename, with_related);

    for (const std::string &file : encrypt_files) {
        Logging::Debug("Encrypting file %s", file.c_str());

        auto [input, output] =
            prepare_streams(file, PrefixParser::apply_prefix(file, prefix, {using_key ? "key" : "pass"}));

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

bool EncryptionVfs::decrypt_file(const std::string &filename, const Encryptor &encryptor, bool with_related,
                                 bool using_key) {
    bool success = true;
    std::vector<std::string> encrypt_files = prepare_files(filename, with_related);

    for (const std::string &file : encrypt_files) {
        if (PrefixParser::contains_prefix(file, prefix)) {
            continue;
        }

        Logging::Debug("Decrypting file %s", file.c_str());

        std::string input_file = PrefixParser::apply_prefix(file, prefix, {using_key ? "key" : "pass"});
        auto [input, output] = prepare_streams(input_file, file);

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

bool EncryptionVfs::encrypt_directory(const std::string &root_directory, const Encryptor &encryptor, bool using_key) {
    std::stack<std::string> directories;
    directories.push(root_directory);
    bool success = true;

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
                success &= encrypt_file(full_path, encryptor, false, using_key);
            }
        }

        CustomVfs::mknod(PrefixParser::apply_prefix(current_directory, prefix), S_IFDIR | 0755, 0);
    }

    return success;
}

bool EncryptionVfs::decrypt_directory(const std::string &root_directory, const Encryptor &encryptor, bool using_key) {
    std::stack<std::string> directories;
    directories.push(root_directory);

    bool success = true;

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
                success &= decrypt_file(full_path, encryptor, false, using_key);
            }
        }

        CustomVfs::unlink(PrefixParser::apply_prefix(current_directory, prefix));
    }

    return success;
}

bool EncryptionVfs::get_default_key_encryptor(Encryptor &encryptor) {
    if (!get_wrapped().exists(Config::encryption.path_to_key_path)) {
        Logging::Error("Key file does not exist");
        return false;
    }

    auto path_stream = CustomVfs::get_ifstream(Config::encryption.path_to_key_path, std::ios::binary);
    std::string key_path;
    *path_stream >> key_path;
    path_stream->close();

    try {
        encryptor = Encryptor::from_file(key_path);
    } catch (const std::exception &e) {
        Logging::Error("Failed to load key file: %s", e.what());
        return false;
    }

    return true;
}

bool EncryptionVfs::is_hook(const std::string &basicString) {
    if (!PrefixParser::contains_prefix(basicString, prefix)) {
        return false;
    }
    return true;
}
