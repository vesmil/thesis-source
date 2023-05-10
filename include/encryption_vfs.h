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

    /// Handles encryption hooks - returns whether it was hook (reference tells wheter it wass successful)
    bool handle_hook(const std::string &path, const std::string &content);

    bool encrypt_file(const std::string &filename, Encryptor &encryptor, bool with_related, bool using_key);
    bool decrypt_file(const std::string &filename, Encryptor &encryptor, bool with_related, bool using_key);

    bool encrypt_directory(const std::string &directory, Encryptor &encryptor, bool using_key);
    bool decrypt_directory(const std::string &directory, Encryptor &encryptor, bool using_key);

    bool get_default_key_encryptor(Encryptor &encryptor);

    std::vector<std::string> prepare_files(const std::string &filename, bool with_related);

    std::pair<std::unique_ptr<std::ifstream>, std::unique_ptr<std::ofstream>> open_files(
        const std::string &input_file, const std::string &output_file, std::ios_base::openmode input_mode,
        std::ios_base::openmode output_mode);
    bool is_hook(const std::string &basicString);
};

#endif  // SRC_ENCRYPTION_VFS_H
