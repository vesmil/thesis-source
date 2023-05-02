#include "encryption_vfs.h"

#include <iostream>

#include "config.h"
#include "custom_vfs.h"

EncryptionVfs::EncryptionVfs(CustomVfs &wrapped_vfs) : VfsDecorator(wrapped_vfs) {
    if (sodium_init() == -1) {
        throw std::runtime_error("Sodium failed to initialize");
    }
}

int EncryptionVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    if (Config::encryption.temporaryCache) {
        // TODO decrypt the file

        return wrapped_vfs.read(pathname, buf, count, offset, fi);
    }

    // TODO read from a temporary file

    return wrapped_vfs.read(pathname, buf, count, offset, fi);
}

int EncryptionVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    std::vector<char> encrypted_buf(count);

    if (Config::encryption.temporaryCache) {
        // TODO encrypt the file

        return wrapped_vfs.write(pathname, encrypted_buf.data(), count, offset, fi);
    }

    // TODO store it into a temporary file

    return wrapped_vfs.write(pathname, buf, count, offset, fi);
}