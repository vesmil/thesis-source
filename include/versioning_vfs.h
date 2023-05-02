#ifndef SRC_VERSIONING_VFS_H
#define SRC_VERSIONING_VFS_H

#include "vfs_decorator.h"

class VersioningVfs : public VfsDecorator {
public:
    explicit VersioningVfs(CustomVfs &wrapped_vfs) : VfsDecorator(wrapped_vfs) {}

    /// Gets the attribute of the highest version
    int getattr(const std::string &pathname, struct stat *st) override;

    /// Writes into a new file
    int write(const std::string &pathname, const char *buf, size_t count, off_t offset,
              struct fuse_file_info *fi) override;

    /// Read from the latest version
    int read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) override;

    /// Hide the last version
    int unlink(const std::string &pathname) override;

    /// Hides from readdir if a versioned
    int fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                 FuseWrapper::fill_dir_flags flags) override;

    [[nodiscard]] std::vector<std::string> subfiles(const std::string &pathname) const override;

private:
    std::string const version_suffix = "#v";

    int get_max_version(const std::string &pathname);

    bool handle_hook(const std::string &pathname, struct fuse_file_info *fi);

    std::vector<std::string> list_versions(const std::string &pathname);
    void restore_version(const std::string &pathname, int version);
    void delete_version(const std::string &pathname, int version);

    bool handle_versioned_command(const std::string &command, const std::string &subArg, const std::string &arg,
                                  const std::string &pathname, struct fuse_file_info *fi);

    bool handle_non_versioned_command(const std::string &command, const std::string &arg, const std::string &pathname,
                                      struct fuse_file_info *fi);
};

#endif  // SRC_VERSIONING_VFS_H
