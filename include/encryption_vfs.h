#ifndef SRC_ENCRYPTION_VFS_H
#define SRC_ENCRYPTION_VFS_H

#include "sodium.h"
#include "vfs_decorator.h"

class EncryptionVfs : public VfsDecorator {
public:
    explicit EncryptionVfs(CustomVfs &wrapped_vfs);

    int read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) override;
    /*
        // Hides encrypted
        int getattr(const std::string &pathname, struct stat *st) override;

        // Checks if directory is encrypted to show warning?
        int readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi, readdir_flags flags) override;

        // Hides encrypted
        int fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                     FuseWrapper::fill_dir_flags flags) override;
    */
    int open(const std::string &pathname, struct fuse_file_info *fi) override;
    int release(const std::string &pathname, struct fuse_file_info *fi) override;

private:
    bool handle_hook(const std::string &path, const std::string &content, fuse_file_info *fi);

    static void derive_key_and_nonce(const std::string &password, unsigned char *key, unsigned char *nonce);
    bool encrypt_file(const std::string &input_filename, const std::string &output_filename,
                      const std::string &password);
    bool decrypt_file(const std::string &input_filename, const std::string &output_filename,
                      const std::string &password);
};

#endif  // SRC_ENCRYPTION_VFS_H