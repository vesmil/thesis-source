#ifndef SRC_VFS_DECORATOR_H
#define SRC_VFS_DECORATOR_H

#include <memory>

#include "custom_vfs.h"

/**
 * @summary A decorator for a CustomVfs that allows to add functionality to the wrapped VFS while keeping the same
 * interface.
 *
 * @see https://refactoring.guru/design-patterns/decorator/cpp/example
 */
class VfsDecorator : public CustomVfs {
public:
    explicit VfsDecorator(CustomVfs &wrapped_vfs) : CustomVfs(wrapped_vfs), vfs_(wrapped_vfs) {}

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

    int symlink(const std::string &target, const std::string &linkpath) override;
    int readlink(const std::string &pathname, char *buf, size_t size) override;
    int link(const std::string &oldpath, const std::string &newpath) override;
    int unlink(const std::string &pathname) override;

    int mkdir(const std::string &pathname, mode_t mode) override;
    int rmdir(const std::string &pathname) override;
    int readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi, readdir_flags flags) override;
    int fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                 FuseWrapper::fill_dir_flags flags) override;
    int opendir(const std::string &pathname, struct fuse_file_info *fi) override;
    int releasedir(const std::string &pathname, struct fuse_file_info *fi) override;

protected:
    [[nodiscard]] CustomVfs &get_wrapped() {
        return vfs_;
    }

    [[nodiscard]] const CustomVfs &get_wrapped() const {
        return vfs_;
    }

private:
    CustomVfs &vfs_;
};

#endif  // SRC_VFS_DECORATOR_H
