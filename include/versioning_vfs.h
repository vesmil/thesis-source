#ifndef SRC_VERSIONING_VFS_H
#define SRC_VERSIONING_VFS_H

#include "common/config.h"
#include "vfs_decorator.h"

/**
 * VFS decorator that supports versioning
 *
 * The main idea behind that is whenever a file is written, a copy of the file is created with a version number (version
 * file) The version number is the maximum version number of the file + 1 and is store using PrefixParser
 */
class VersioningVfs : public VfsDecorator {
public:
    explicit VersioningVfs(CustomVfs &wrapped_vfs) : VfsDecorator(wrapped_vfs) {}

    // Creates a copy of the file with the version number (version file) and handles hooks
    int write(const std::string &pathname, const char *buf, size_t count, off_t offset,
              struct fuse_file_info *fi) override;

    // Hides files for versioning
    int fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                 FuseWrapper::fill_dir_flags flags) override;

    // Hides files for versioning
    [[nodiscard]] std::vector<std::string> subfiles(const std::string &pathname) const override;

protected:
    [[nodiscard]] std::vector<std::string> get_related_files(const std::string &pathname) const override;

private:
    /// @brief Prefix for the version files used by PrefixParser
    std::string const prefix = Config::versioning.prefix;

    /// @brief Lists all version file names corresponding to a non-prefixed path
    [[nodiscard]] std::vector<std::string> get_related_names(const std::string &pathname) const;

    /// @brief Checks whether path corresponds to a version file
    [[nodiscard]] bool is_version_file(const std::string &pathname) const;

    /// @brief Get maximum version for non-prefix path
    [[nodiscard]] int get_max_version(const std::string &pathname);

    /// @brief Handle versioning hooks
    bool handle_hook(const std::string &pathname);

    /// @brief Sets previous version as the current
    void restore_version(const std::string &pathname, int version);

    /// @brief Deletes a version file
    void delete_version(const std::string &pathname, int version);

    /// @brief Handles hook with version number
    bool handle_versioned_command(const std::string &command, const std::string &subArg, const std::string &arg_path,
                                  [[maybe_unused]] const std::string &hook_file);

    /// @brief Handles hook without version number
    bool handle_non_versioned_command(const std::string &command, const std::string &arg_path,
                                      const std::string &hook_file);

    /// @brief Deletes all versions of a file
    void delete_all_versions(const std::string &base_name);
    void list_versions(const std::string &arg_path, const std::string &hook_file);
};

#endif  // SRC_VERSIONING_VFS_H
