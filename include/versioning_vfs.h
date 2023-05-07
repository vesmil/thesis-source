#ifndef SRC_VERSIONING_VFS_H
#define SRC_VERSIONING_VFS_H

#include "vfs_decorator.h"

/**
 * VFS decorator that supports versioning
 *
 * TODO explain high-level how it works
 */
class VersioningVfs : public VfsDecorator {
public:
    explicit VersioningVfs(CustomVfs &wrapped_vfs) : VfsDecorator(wrapped_vfs) {}

    /// Writes into a new file
    int write(const std::string &pathname, const char *buf, size_t count, off_t offset,
              struct fuse_file_info *fi) override;

    /// Just hides the last version, also allows deleting folder
    int unlink(const std::string &pathname) override;

    /// Hides from readdir if a version file
    int fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                 FuseWrapper::fill_dir_flags flags) override;

    [[nodiscard]] std::vector<std::string> subfiles(const std::string &pathname) const override;

protected:
    [[nodiscard]] std::vector<std::string> get_related_files(const std::string &pathname) const override;

private:
    std::string const prefix = "VERSION";

    [[nodiscard]] bool is_version_file(const std::string &pathname) const;
    int get_max_version(const std::string &pathname);

    bool handle_hook(const std::string &pathname, struct fuse_file_info *fi);

    [[nodiscard]] std::vector<std::string> list_helper_files(const std::string &pathname) const;

    void restore_version(const std::string &pathname, int version);
    void delete_version(const std::string &pathname, int version);

    bool handle_versioned_command(const std::string &command, const std::string &subArg, const std::string &arg_path,
                                  const std::string &pathname, struct fuse_file_info *fi);

    bool handle_non_versioned_command(const std::string &command, const std::string &arg_path,
                                      const std::string &pathname, struct fuse_file_info *fi);
    void delete_all_versions(const std::string &base_name);
};

#endif  // SRC_VERSIONING_VFS_H
