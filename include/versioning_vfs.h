#ifndef SRC_VERSIONING_VFS_H
#define SRC_VERSIONING_VFS_H

#include "vfs_decorator.h"

class VersioningVfs : public VfsDecorator {
public:
    explicit VersioningVfs(CustomVfs &wrapped_vfs) : VfsDecorator(wrapped_vfs) {}

    int write(const std::string &pathname, const char *buf, size_t count, off_t offset,
              struct fuse_file_info *fi) override;

    int read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) override;

private:
    bool handle_hook(int &result, const std::string &pathname, char *buf, size_t count, off_t offset,
                     struct fuse_file_info *fi);

    std::vector<std::string> list_versions(const std::string &pathname);
    void restore_version(const std::string &pathname, int version);
    void delete_version(const std::string &pathname, int version);
};

#endif  // SRC_VERSIONING_VFS_H
