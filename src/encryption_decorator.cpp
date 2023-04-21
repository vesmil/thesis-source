#include "encryption_decorator.h"

#include <iostream>

#include "custom_vfs.h"

EncryptionVfs::EncryptionVfs(CustomVfs &wrapped_vfs) : VfsDecorator(wrapped_vfs) {
    if (sodium_init() == -1) {
        throw std::runtime_error("Sodium failed to initialize");
    }
}

int EncryptionVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    int result = wrapped_vfs_.read(pathname, buf, count, offset, fi);

    if (result >= 0) {
    }

    return result;
}

int EncryptionVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    std::vector<char> encrypted_buf(count);

    return wrapped_vfs_.write(pathname, encrypted_buf.data(), count, offset, fi);
}