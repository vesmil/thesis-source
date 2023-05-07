#include "vfs_decorator.h"

int VfsDecorator::mknod(const std::string &pathname, mode_t mode, dev_t dev) {
    return vfs_.mknod(pathname, mode, dev);
}

int VfsDecorator::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    return vfs_.read(pathname, buf, count, offset, fi);
}

int VfsDecorator::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                        struct fuse_file_info *fi) {
    return vfs_.write(pathname, buf, count, offset, fi);
}

int VfsDecorator::truncate(const std::string &pathname, off_t length) {
    return vfs_.truncate(pathname, length);
}

int VfsDecorator::rename(const std::string &oldpath, const std::string &newpath, unsigned int flags) {
    return vfs_.rename(oldpath, newpath, flags);
}

int VfsDecorator::getattr(const std::string &pathname, struct stat *st) {
    return vfs_.getattr(pathname, st);
}

int VfsDecorator::statfs(const std::string &pathname, struct statvfs *stbuf) {
    return vfs_.statfs(pathname, stbuf);
}

int VfsDecorator::utimens(const std::string &pathname, const struct timespec tv[2]) {
    return vfs_.utimens(pathname, tv);
}

int VfsDecorator::chmod(const std::string &pathname, mode_t mode) {
    return vfs_.chmod(pathname, mode);
}

int VfsDecorator::open(const std::string &pathname, struct fuse_file_info *fi) {
    return vfs_.open(pathname, fi);
}

int VfsDecorator::release(const std::string &pathname, struct fuse_file_info *fi) {
    return vfs_.release(pathname, fi);
}

int VfsDecorator::symlink(const std::string &target, const std::string &linkpath) {
    return vfs_.symlink(target, linkpath);
}

int VfsDecorator::readlink(const std::string &pathname, char *buf, size_t size) {
    return vfs_.readlink(pathname, buf, size);
}

int VfsDecorator::link(const std::string &oldpath, const std::string &newpath) {
    return vfs_.link(oldpath, newpath);
}

int VfsDecorator::unlink(const std::string &pathname) {
    return vfs_.unlink(pathname);
}

int VfsDecorator::mkdir(const std::string &pathname, mode_t mode) {
    return vfs_.mkdir(pathname, mode);
}

int VfsDecorator::rmdir(const std::string &pathname) {
    return vfs_.rmdir(pathname);
}

int VfsDecorator::readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi, readdir_flags flags) {
    return vfs_.readdir(pathname, off, fi, flags);
}

int VfsDecorator::fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                           FuseWrapper::fill_dir_flags flags) {
    return vfs_.fill_dir(name, stbuf, off, flags);
}

int VfsDecorator::opendir(const std::string &pathname, struct fuse_file_info *fi) {
    return vfs_.opendir(pathname, fi);
}

int VfsDecorator::releasedir(const std::string &pathname, struct fuse_file_info *fi) {
    return vfs_.releasedir(pathname, fi);
}