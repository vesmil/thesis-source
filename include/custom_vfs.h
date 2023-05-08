#ifndef SRC_CUSTOM_VFS_H
#define SRC_CUSTOM_VFS_H

#include <string>
#include <vector>

#include "common/path.h"
#include "fuse_wrapper.h"

/**
 * A custom filesystem based on storing files into backing folder
 *
 * It uses the FuseWrapper to provide a FUSE interface
 */
class CustomVfs : public FuseWrapper {
public:
    /**
     * Create a new filesystem from a given directory
     * The directory will be copied into the filesystem
     *
     * @param path The path to the directory
     * @param create_test Whether to create example files
     */
    explicit CustomVfs(const std::string &path, const std::string &backing = "");

    void init() override;
    void destroy() override;

    // Files
    int mknod(const std::string &pathname, mode_t mode, dev_t dev) override;
    int read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) override;
    int write(const std::string &pathname, const char *buf, size_t count, off_t offset,
              struct fuse_file_info *fi) override;
    int truncate(const std::string &pathname, off_t length) override;
    int rename(const std::string &oldpath, const std::string &newpath, unsigned int flags) override;
    int getattr(const std::string &pathname, struct stat *st) override;
    int statfs(const std::string &pathname, struct statvfs *stbuf) override;
    int utimens(const std::string &pathname, const struct timespec tv[2]) override;
    int chmod(const std::string &pathname, mode_t mode) override;
    int open(const std::string &pathname, struct fuse_file_info *fi) override;
    int release(const std::string &pathname, struct fuse_file_info *fi) override;
    int flush(const std::string &pathname, struct fuse_file_info *fi) override;

    // Links
    int symlink(const std::string &target, const std::string &linkpath) override;
    int readlink(const std::string &pathname, char *buf, size_t size) override;
    int link(const std::string &oldpath, const std::string &newpath) override;
    int unlink(const std::string &pathname) override;

    // Directories
    int mkdir(const std::string &pathname, mode_t mode) override;
    int rmdir(const std::string &pathname) override;
    int readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi, readdir_flags flags) override;
    virtual int fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                         FuseWrapper::fill_dir_flags flags);
    int opendir(const std::string &pathname, struct fuse_file_info *fi) override;
    int releasedir(const std::string &pathname, struct fuse_file_info *fi) override;

    // Misc
    [[nodiscard]] std::string get_fs_path(const std::string &pathname) const;

    [[nodiscard]] std::unique_ptr<std::ofstream> get_ofstream(const std::string &path,
                                                              std::ios_base::openmode mode) const;
    [[nodiscard]] std::unique_ptr<std::ifstream> get_ifstream(const std::string &path,
                                                              std::ios_base::openmode mode) const;

    [[nodiscard]] virtual std::vector<std::string> get_related_files(const std::string &pathname) const;

    int copy_file(const std::string &source, const std::string &destination);

    [[nodiscard]] virtual std::vector<std::string> subfiles(const std::string &pathname) const;
    [[nodiscard]] bool is_directory(const std::string &pathname) const;
    [[nodiscard]] bool exists(const std::string &pathname) const;

private:
    // Converts a path to its real path in the backing directory
    [[nodiscard]] std::string to_backing(const std::string &pathname) const;
    [[nodiscard]] static Path initial_backing_path(const std::string &backing, const std::string &vfs_name);

    template <typename Func, typename... Args>
    static int posix_call_result(Func operation, Args... args) {
        int result = operation(args...);
        if (result < 0) {
            return -errno;
        }
        return result;
    }

    Path backing_dir;
    const Path mount_path;
};

#endif
