#ifndef SRC_ENCRYPTION_DECORATOR_H
#define SRC_ENCRYPTION_DECORATOR_H

#include "sodium.h"
#include "vfs_decorator.h"

class EncryptionVfs : public VfsDecorator {
public:
    explicit EncryptionVfs(CustomVfs &wrapped_vfs);

    int read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) override;
    int write(const std::string &pathname, const char *buf, size_t count, off_t offset,
              struct fuse_file_info *fi) override;
};

#endif  // SRC_ENCRYPTION_DECORATOR_H