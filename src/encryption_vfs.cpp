#include "encryption_vfs.h"

#include <iostream>

#include "custom_vfs.h"

EncryptionVfs::EncryptionVfs(CustomVfs &wrapped_vfs) : VfsDecorator(wrapped_vfs) {
    if (sodium_init() == -1) {
        throw std::runtime_error("Sodium failed to initialize");
    }
}

int EncryptionVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    int result = wrapped_vfs.read(pathname, buf, count, offset, fi);

    // TODO store a cache...

    if (result >= 0) {}

    return result;
}

int EncryptionVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    // TODO should it be writing straight to the file or to a cache?

    std::vector<char> encrypted_buf(count);

    return wrapped_vfs.write(pathname, encrypted_buf.data(), count, offset, fi);
}