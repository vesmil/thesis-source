#include "custom_vfs.h"

#include <dirent.h>
#include <fcntl.h>
#include <fuse_lowlevel.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include <cerrno>
#include <filesystem>
#include <fstream>

#include "config.h"

CustomVfs::CustomVfs(const std::string &path, bool test) : mount_path(path), create_test(test) {
    std::string parent = CustomVfs::parent_path(path);
    std::string name = CustomVfs::toplevel_name(path);

    backing_dir = parent + Config::base.backing_prefix + name;
    std::filesystem::create_directory(backing_dir);
}

void CustomVfs::init() {
    FuseWrapper::init();

    if (create_test) {
        std::filesystem::create_directory(mount_path + "/dir");
        std::string filename = mount_path + "/dir/file.txt";
        int fd = ::open(filename.c_str(), O_CREAT | O_RDWR);
        ::write(fd, "Hello World", 11);
        ::close(fd);
    }
}

void CustomVfs::destroy() {}

int CustomVfs::mknod(const std::string &pathname, mode_t mode, dev_t dev) {
    std::string real_path = to_backing(pathname);
    return ::mknod(real_path.c_str(), mode, dev);
}

int CustomVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    std::string real_path = to_backing(pathname);
    int fd = ::open(real_path.c_str(), O_RDONLY);
    if (fd < 0) {
        return -errno;
    }
    int ret = ::pread(fd, buf, count, offset);
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
    int ret = ::pwrite(fd, buf, count, offset);
    ::close(fd);
    return ret;
}

int CustomVfs::truncate(const std::string &pathname, off_t length) {
    std::string real_path = to_backing(pathname);
    return ::truncate(real_path.c_str(), length);
}

int CustomVfs::rename(const std::string &oldpath, const std::string &newpath, unsigned int flags) {
    std::string old_real_path = to_backing(oldpath);
    std::string new_real_path = to_backing(newpath);
    return ::renameat(AT_FDCWD, old_real_path.c_str(), AT_FDCWD, new_real_path.c_str());
}

int CustomVfs::getattr(const std::string &pathname, struct stat *st) {
    std::string real_path = to_backing(pathname);
    int result = ::lstat(real_path.c_str(), st);
    if (result < 0) {
        return -errno;
    }
    return result;
}

int CustomVfs::statfs(const std::string &pathname, struct statvfs *stbuf) {
    std::string real_path = to_backing(pathname);
    return ::statvfs(real_path.c_str(), stbuf);
}

int CustomVfs::utimens(const std::string &pathname, const struct timespec tv[2]) {
    std::string real_path = to_backing(pathname);
    return ::utimensat(AT_FDCWD, real_path.c_str(), tv, AT_SYMLINK_NOFOLLOW);
}

int CustomVfs::chmod(const std::string &pathname, mode_t mode) {
    std::string real_path = to_backing(pathname);
    return ::chmod(real_path.c_str(), mode);
}

int CustomVfs::open(const std::string &pathname, struct fuse_file_info *fi) {
    std::string real_path = to_backing(pathname);
    int fd = ::open(real_path.c_str(), fi->flags);
    if (fd < 0) {
        return -errno;
    }
    fi->fh = fd;
    return 0;
}

int CustomVfs::symlink(const std::string &target, const std::string &linkpath) {
    std::string real_linkpath = to_backing(linkpath);
    return ::symlink(target.c_str(), real_linkpath.c_str());
}

int CustomVfs::readlink(const std::string &pathname, char *buf, size_t size) {
    std::string real_path = to_backing(pathname);
    return ::readlink(real_path.c_str(), buf, size);
}

int CustomVfs::link(const std::string &oldpath, const std::string &newpath) {
    std::string old_real_path = to_backing(oldpath);
    std::string new_real_path = to_backing(newpath);
    return ::link(old_real_path.c_str(), new_real_path.c_str());
}

int CustomVfs::unlink(const std::string &pathname) {
    std::string real_path = to_backing(pathname);
    return ::unlink(real_path.c_str());
}

int CustomVfs::mkdir(const std::string &pathname, mode_t mode) {
    std::string real_path = to_backing(pathname);
    return ::mkdir(real_path.c_str(), mode);
}

int CustomVfs::rmdir(const std::string &pathname) {
    std::string real_path = to_backing(pathname);
    return ::rmdir(real_path.c_str());
}

int CustomVfs::readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi, readdir_flags flags) {
    std::string real_path = to_backing(pathname);

    std::ofstream log_file("/home/vesmil/log.txt", std::ios::app);  // Open the file in append mode

    if (log_file.is_open()) {
        log_file << "readdir: " << real_path << std::endl;
    }

    struct stat stbuf {};
    if (::lstat(real_path.c_str(), &stbuf) != 0 || !S_ISDIR(stbuf.st_mode)) {
        log_file << "readdir: " << real_path << " is not a directory" << std::endl;
        log_file.close();
        return -ENOENT;
    }

    // Iterate over directory entries
    off_t current_offset = 0;
    for (const auto &entry : std::filesystem::directory_iterator(real_path)) {
        log_file << "entry: " << entry.path() << std::endl;
        if (current_offset >= off) {
            // Get entry attributes
            struct stat entry_stbuf {};
            if (::lstat(entry.path().c_str(), &entry_stbuf) == 0) {
                // Fill the directory entry
                log_file << "fill_dir: " << entry.path().filename() << std::endl;
                if (fill_dir(entry.path().filename(), &entry_stbuf, current_offset + 1, FILL_DIR_PLUS) == 1) {
                    break;
                }
            }
        }
        current_offset++;
        log_file << "current_offset: " << current_offset << std::endl;
    }

    log_file << "Closing file" << std::endl;  // "Close the file
    log_file.close();
    return 0;
}

int CustomVfs::opendir(const std::string &pathname, struct fuse_file_info *fi) {
    std::string real_path = to_backing(pathname);
    DIR *dp = ::opendir(real_path.c_str());
    if (dp == nullptr) {
        return -errno;
    }
    fi->fh = reinterpret_cast<uintptr_t>(dp);
    return 0;
}

int CustomVfs::releasedir(const std::string &pathname, struct fuse_file_info *fi) {
    DIR *dp = reinterpret_cast<DIR *>(fi->fh);
    if (::closedir(dp) < 0) {
        return -errno;
    }
    return 0;
}

std::vector<std::string> CustomVfs::subfiles(const std::string &pathname) const {
    std::string real_path = to_backing(pathname);
    std::vector<std::string> files;

    for (const auto &entry : std::filesystem::directory_iterator(real_path)) {
        files.push_back(entry.path().filename());
    }

    return files;
}

int CustomVfs::release(const std::string &pathname, struct fuse_file_info *fi) { return ::close(fi->fh); }

std::string CustomVfs::parent_path(const std::string &path) { return path.substr(0, path.rfind('/')); }

std::string CustomVfs::toplevel_name(const std::string &basicString) {
    return basicString.substr(basicString.rfind('/') + 1);
}

std::string CustomVfs::to_backing(const std::string &pathname) const { return backing_dir + pathname; }