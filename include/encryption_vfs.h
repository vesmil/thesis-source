#ifndef SRC_ENCRYPTION_VFS_H
#define SRC_ENCRYPTION_VFS_H

#include "sodium.h"
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
    std::string const prefix = "ENCRYPTION";

    /// Checks whether path corresponds to an encrypted file
    [[nodiscard]] bool is_encrypted(const std::string &pathname) const;

    /// Handles encryption hooks
    bool handle_hook(const std::string &path, const std::string &content, fuse_file_info *fi);

    bool encrypt_file(const std::string &filename, const std::string &password, bool with_related = true);
    bool decrypt_file(const std::string &filename, const std::string &password, bool with_related = true);

    void encrypt_directory(const std::string &directory, const std::string &password);
    void decrypt_directory(const std::string &directory, const std::string &password);
};

#endif  // SRC_ENCRYPTION_VFS_H