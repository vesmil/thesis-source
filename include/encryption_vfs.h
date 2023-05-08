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

    int read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) override;
    int write(const std::string &pathname, const char *buf, size_t count, off_t offset,
              struct fuse_file_info *fi) override;

    int readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi, readdir_flags flags) override;

    int open(const std::string &pathname, struct fuse_file_info *fi) override;
    int release(const std::string &pathname, struct fuse_file_info *fi) override;

private:
    std::string const prefix = "ENCRYPTION";

    [[nodiscard]] bool is_encrypted(const std::string &pathname) const;
    bool handle_hook(const std::string &path, const std::string &content, fuse_file_info *fi);

    bool encrypt_file(const std::string &filename, const std::string &password);
    bool decrypt_file(const std::string &filename, const std::string &password);
    void encrypt_directory(const std::string &directory, const std::string &password);
    void encrypt_filename(const std::string &filename, const std::string &password);

    bool directory_command(const std::string &command, const std::string &path, const std::string &pass);
    void decrypt_directory(const std::string &directory, const std::string &password);
    void decrypt_filename(const std::string &filename, const std::string &password);
};

#endif  // SRC_ENCRYPTION_VFS_H