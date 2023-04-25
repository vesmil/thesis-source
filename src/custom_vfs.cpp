#include "custom_vfs.h"

#include <dirent.h>
#include <fcntl.h>
#include <fuse_lowlevel.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include <cerrno>
#include <filesystem>

#include "config.h"

CustomVfs::CustomVfs(const std::string &path) : mount_path(path) {
    std::string parent = CustomVfs::get_parent(path);
    std::string name = CustomVfs::get_filename(path);

    backing_dir = parent + Config::base.backing_prefix + name;
    std::filesystem::create_directory(backing_dir);
}

void CustomVfs::init() {
    FuseWrapper::init();
}

void CustomVfs::destroy() {}

int CustomVfs::mknod(const std::string &pathname, mode_t mode, dev_t dev) {
    return posix_call_result(::mknod, to_backing(pathname).c_str(), mode, dev);
}

int CustomVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    int fd = ::open(to_backing(pathname).c_str(), O_RDONLY);
    if (fd < 0) {
        return -errno;
    }
    int ret = static_cast<int>(::pread(fd, buf, count, offset));
    ::close(fd);
    return ret;
}

int CustomVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                     struct fuse_file_info *fi) {
    std::string real_path = to_backing(pathname);
    int fd = ::open(real_path.c_str(), O_WRONLY);
    if (fd < 0) {
        return -errno;
    }
    int ret = static_cast<int>(::pwrite(fd, buf, count, offset));
    ::close(fd);
    return ret;
}

int CustomVfs::truncate(const std::string &pathname, off_t length) {
    return posix_call_result(::truncate, to_backing(pathname).c_str(), length);
}

int CustomVfs::rename(const std::string &oldpath, const std::string &newpath, unsigned int flags) {
    std::string old_real_path = to_backing(oldpath);
    std::string new_real_path = to_backing(newpath);
    return posix_call_result(::rename, old_real_path.c_str(), new_real_path.c_str());
}

int CustomVfs::getattr(const std::string &pathname, struct stat *st) {
    return posix_call_result(::lstat, to_backing(pathname).c_str(), st);
}

int CustomVfs::statfs(const std::string &pathname, struct statvfs *stbuf) {
    return posix_call_result(::statvfs, to_backing(pathname).c_str(), stbuf);
}

int CustomVfs::utimens(const std::string &pathname, const struct timespec tv[2]) {
    return posix_call_result(::utimensat, AT_FDCWD, to_backing(pathname).c_str(), tv, AT_SYMLINK_NOFOLLOW);
}

int CustomVfs::chmod(const std::string &pathname, mode_t mode) {
    return posix_call_result(::chmod, to_backing(pathname).c_str(), mode);
}

int CustomVfs::open(const std::string &pathname, struct fuse_file_info *fi) {
    int fd = ::open(to_backing(pathname).c_str(), fi->flags);
    if (fd < 0) {
        return -errno;
    }
    fi->fh = fd;
    return 0;
}

int CustomVfs::symlink(const std::string &target, const std::string &linkpath) {
    return posix_call_result(::symlink, target.c_str(), to_backing(linkpath).c_str());
}

int CustomVfs::readlink(const std::string &pathname, char *buf, size_t size) {
    return posix_call_result(::readlink, to_backing(pathname).c_str(), buf, size);
}

int CustomVfs::link(const std::string &oldpath, const std::string &newpath) {
    std::string old_real_path = to_backing(oldpath);
    std::string new_real_path = to_backing(newpath);
    return posix_call_result(::link, old_real_path.c_str(), new_real_path.c_str());
}

int CustomVfs::unlink(const std::string &pathname) {
    return posix_call_result(::unlink, to_backing(pathname).c_str());
}

int CustomVfs::mkdir(const std::string &pathname, mode_t mode) {
    return posix_call_result(::mkdir, to_backing(pathname).c_str(), mode);
}

int CustomVfs::rmdir(const std::string &pathname) {
    return posix_call_result(::rmdir, to_backing(pathname).c_str());
}

int CustomVfs::opendir(const std::string &pathname, struct fuse_file_info *fi) {
    DIR *dp = ::opendir(to_backing(pathname).c_str());
    if (dp == nullptr) {
        return -errno;
    }
    fi->fh = reinterpret_cast<uintptr_t>(dp);
    return 0;
}

int CustomVfs::releasedir(const std::string &pathname, struct fuse_file_info *fi) {
    DIR *dp = reinterpret_cast<DIR *>(fi->fh);
    return posix_call_result(::closedir, dp);
}

int CustomVfs::release(const std::string &pathname, struct fuse_file_info *fi) {
    return posix_call_result(::close, static_cast<int>(fi->fh));
}

int CustomVfs::readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi, readdir_flags flags) {
    std::string real_path = to_backing(pathname);
    struct stat stbuf {};
    if (::lstat(real_path.c_str(), &stbuf) != 0 || !S_ISDIR(stbuf.st_mode)) {
        return -ENOENT;
    }

    off_t current_offset = 0;
    for (const auto &entry : std::filesystem::directory_iterator(real_path)) {
        if (current_offset >= off) {
            struct stat entry_stbuf {};
            if (::lstat(entry.path().c_str(), &entry_stbuf) == 0) {
                if (fill_dir(entry.path().filename(), &entry_stbuf, current_offset + 1, FILL_DIR_PLUS) == 1) {
                    break;
                }
            }
        }
        current_offset++;
    }

    return 0;
}

std::vector<std::string> CustomVfs::subfiles(const std::string &pathname) const {
    std::string real_path = to_backing(pathname);
    std::vector<std::string> files;

    for (const auto &entry : std::filesystem::directory_iterator(real_path)) {
        std::string path = entry.path();
        path.replace(0, backing_dir.length(), mount_path);
        files.push_back(path);
    }

    return files;
}

std::string CustomVfs::get_parent(const std::string &path) {
    return path.substr(0, path.rfind('/'));
}

std::string CustomVfs::get_filename(const std::string &basicString) {
    return basicString.substr(basicString.rfind('/') + 1);
}

std::string CustomVfs::to_backing(const std::string &pathname) const {
    return backing_dir + pathname;
}