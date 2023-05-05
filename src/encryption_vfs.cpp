#include "encryption_vfs.h"

#include <sodium.h>

#include <iostream>

#include "config.h"
#include "custom_vfs.h"

EncryptionVfs::EncryptionVfs(CustomVfs &wrapped_vfs) : VfsDecorator(wrapped_vfs) {
    if (sodium_init() == -1) {
        throw std::runtime_error("Sodium failed to initialize");
    }
}

int EncryptionVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    if (handle_hook(pathname, fi)) {
        return 0;
    }

    return CustomVfs::read(pathname, buf, count, offset, fi);
}

bool EncryptionVfs::handle_hook(const std::string &path, fuse_file_info *fi) {
    if (path[0] == '#') {
        // TODO

        return true;
    }

    return false;
}

int EncryptionVfs::open(const std::string &pathname, struct fuse_file_info *fi) {
    auto realPath = get_fs_path(pathname);

    return CustomVfs::open(pathname, fi);
}
int EncryptionVfs::release(const std::string &pathname, struct fuse_file_info *fi) {
    return CustomVfs::release(pathname, fi);
}