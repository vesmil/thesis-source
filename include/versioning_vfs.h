#ifndef SRC_VERSIONING_VFS_H
#define SRC_VERSIONING_VFS_H

#include "vfs_decorator.h"

class VersioningVfs : public VfsDecorator {
public:
    explicit VersioningVfs(CustomVfs &wrapped_vfs) : VfsDecorator(wrapped_vfs) {}

    int write(const std::string &pathname, const char *buf, size_t count, off_t offset,
              struct fuse_file_info *fi) override;
    int read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) override;
    int unlink(const std::string &pathname) override;
    int readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi, readdir_flags flags) override;
    int fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                 FuseWrapper::fill_dir_flags flags) override;

    [[nodiscard]] std::vector<std::string> subfiles(const std::string &pathname) const override;

private:
    std::string const version_prefix = "#v";

    int get_max_version(const std::string &pathname);

    bool handle_hook(int &result, const std::string &pathname, char *buf, size_t count, off_t offset,
                     struct fuse_file_info *fi);

    std::vector<std::string> list_versions(const std::string &pathname);
    void restore_version(const std::string &pathname, int version);
    void delete_version(const std::string &pathname, int version);
};

#endif  // SRC_VERSIONING_VFS_H
