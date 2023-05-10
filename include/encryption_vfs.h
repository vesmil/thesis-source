#ifndef SRC_ENCRYPTION_VFS_H
#define SRC_ENCRYPTION_VFS_H

#include <sodium.h>

#include <fstream>
#include <string>

#include "common/config.h"
#include "encryptor.h"
#include "hook-generation/encryption.h"
#include "vfs_decorator.h"

/**
 * EncryptionVfs is a decorator for CustomVfs that encrypts and decrypts files
 *
 * It works by intercepting the read and write calls and encrypting/decrypting the content upon registering hooks
 */
class EncryptionVfs : public VfsDecorator {
public:
    explicit EncryptionVfs(CustomVfs &wrapped_vfs);

    int write(const std::string &pathname, const char *buf, size_t count, off_t offset,
              struct fuse_file_info *fi) override;

    int open(const std::string &pathname, struct fuse_file_info *fi) override;
    int release(const std::string &pathname, struct fuse_file_info *fi) override;

private:
    /// Prefix for the encrypted files used by PrefixParser
    std::string const prefix = Config::encryption.prefix;

    /// Checks whether path corresponds to an encrypted file
    [[nodiscard]] bool is_encrypted(const std::string &pathname) const;

    bool encrypt_file(const std::string &filename, const Encryptor &encryptor, bool with_related, bool using_key);
    bool decrypt_file(const std::string &filename, const Encryptor &encryptor, bool with_related, bool using_key);

    bool encrypt_directory(const std::string &directory, const Encryptor &encryptor, bool using_key);
    bool decrypt_directory(const std::string &directory, const Encryptor &encryptor, bool using_key);

    bool get_default_key_encryptor(Encryptor &encryptor);

    std::vector<std::string> prepare_files(const std::string &filename, bool with_related);

    std::pair<std::unique_ptr<std::ifstream>, std::unique_ptr<std::ofstream>> prepare_streams(
        const std::string &input_file, const std::string &output_file);

    /// Handles encryption hooks - returns whether it was successful
    bool handle_hook(const std::string &path, const std::string &content);
    bool is_hook(const std::string &basicString);

    bool handle_single_arg(const std::string &non_prefixed, const std::string &arg, const std::string &content,
                           bool is_dir);
    bool handle_double_arg(const std::string &non_prefixed, const std::string &arg, const std::string &key_path_arg,
                           bool is_dir);

    bool handle_encryption_action(const std::string &non_prefixed, const std::string &arg, const Encryptor &encryptor,
                                  bool is_dir, bool use_key_file);
    bool generate_encryption_file(const std::string &non_prefixed);
    bool set_default_key(const std::string &key_path_arg);
};

#endif  // SRC_ENCRYPTION_VFS_H
