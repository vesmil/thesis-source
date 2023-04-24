#include "custom_vfs.h"

#include <dirent.h>
#include <fcntl.h>
#include <fuse_lowlevel.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "config.h"

#define LOG(...)                                                    \
    do {                                                            \
        std::fstream stream("/home/vesmil/log.txt", std::ios::app); \
        stream << __VA_ARGS__ << std::endl;                         \
        stream.close();                                             \
    } while (0);

#define POSIX_CALL(call)   \
    LOG(#call)             \
    do {                   \
        int res = call;    \
        if (res == -1) {   \
            return -errno; \
        }                  \
    } while (0);

CustomVfs::CustomVfs(const std::string &path, bool test) : mount_path(path), create_test(test) {
    std::string parent = CustomVfs::parent_path(path);
    std::string name = CustomVfs::toplevel_name(path);

    backing_dir = parent + Config::base.backing_prefix + name;
    std::filesystem::create_directory(backing_dir);
}

void CustomVfs::init() {
    FuseWrapper::init();

    if (create_test) {
        std::filesystem::create_directory(backing_dir + "/dir");
        std::string filename = backing_dir + "/dir/file.txt";
        int fd = ::open(filename.c_str(), O_CREAT | O_RDWR);
        ::write(fd, "Hello World", 11);
        ::close(fd);
    }
}

void CustomVfs::destroy() { FuseWrapper::destroy(); }

int CustomVfs::mknod(const std::string &pathname, mode_t mode, dev_t dev) {
    POSIX_CALL(::mknod(to_backing(pathname).c_str(), mode, dev));
    return 0;
}

int CustomVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    LOG("read " << pathname);
    int fd = fi->fh;
    return ::pread(fd, buf, count, offset);
}

int CustomVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                     struct fuse_file_info *fi) {
    LOG("write " << pathname);
    int fd = fi->fh;
    return ::pwrite(fd, buf, count, offset);
}

int CustomVfs::truncate(const std::string &pathname, off_t length) {
    std::string full_path = to_backing(pathname);
    return ::truncate(full_path.c_str(), length);
}

int CustomVfs::rename(const std::string &oldpath, const std::string &newpath, unsigned int flags) {
    std::string full_old = to_backing(oldpath);
    std::string full_new = to_backing(newpath);

    return ::rename(oldpath.c_str(), newpath.c_str());
}

int CustomVfs::getattr(const std::string &pathname, struct stat *st) {
    return ::lstat(to_backing(pathname).c_str(), st);
}

int CustomVfs::statfs(const std::string &pathname, struct statvfs *stbuf) {
    return ::statvfs(to_backing(pathname).c_str(), stbuf);
}

int CustomVfs::utimens(const std::string &pathname, const struct timespec tv[2]) {
    return ::utimensat(AT_FDCWD, to_backing(pathname).c_str(), tv, AT_SYMLINK_NOFOLLOW);
}

int CustomVfs::chmod(const std::string &pathname, mode_t mode) { return ::chmod(to_backing(pathname).c_str(), mode); }

int CustomVfs::open(const std::string &pathname, struct fuse_file_info *fi) {
    LOG("open " << pathname)
    int fd = ::open(to_backing(pathname).c_str(), fi->flags);
    if (fd == -1) {
        return -errno;
    }
    fi->fh = fd;
    return 0;
}

int CustomVfs::release(const std::string &pathname, struct fuse_file_info *fi) { return ::close(fi->fh); }

int CustomVfs::symlink(const std::string &target, const std::string &linkpath) {
    return ::symlink(to_backing(target).c_str(), to_backing(linkpath).c_str());
}

int CustomVfs::readlink(const std::string &pathname, char *buf, size_t size) {
    return ::readlink(to_backing(pathname).c_str(), buf, size);
}

int CustomVfs::link(const std::string &oldpath, const std::string &newpath) {
    return ::link(oldpath.c_str(), newpath.c_str());
}

int CustomVfs::unlink(const std::string &pathname) { return ::unlink(to_backing(pathname).c_str()); }

int CustomVfs::mkdir(const std::string &pathname, mode_t mode) {
    POSIX_CALL(::mkdir(to_backing(pathname).c_str(), mode));
    return 0;
}

int CustomVfs::rmdir(const std::string &pathname) { return ::rmdir(to_backing(pathname).c_str()); }

int CustomVfs::readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi, readdir_flags flags) {
    auto real_path = to_backing(pathname);

    DIR *dp;
    struct dirent *de;

    dp = ::opendir(to_backing(pathname).c_str());
    if (dp == nullptr) {
        return -errno;
    }

    while ((de = ::readdir(dp)) != nullptr) {
        struct stat st {};
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;

        std::error_code ec;
        auto status = std::filesystem::status(real_path, ec);
        if (ec) {
            LOG("Error getting status for " << real_path << ": " << ec.message());
            continue;
        }
        if (status.type() == std::filesystem::file_type::directory) {
            st.st_mode |= S_IFDIR;
        } else if (status.type() == std::filesystem::file_type::regular) {
            st.st_mode |= S_IFREG;
        } else if (status.type() == std::filesystem::file_type::symlink) {
            st.st_mode |= S_IFLNK;
        } else {
            LOG("Unknown file type for " << real_path);
            continue;
        }

        // ...

        if (fill_dir(de->d_name, &st, flags)) {
            break;
        }
    }

    ::closedir(dp);
    return 0;
}

std::string CustomVfs::parent_path(const std::string &path) { return path.substr(0, path.rfind('/')); }

std::string CustomVfs::toplevel_name(const std::string &basicString) {
    return basicString.substr(basicString.rfind('/') + 1);
}

std::vector<std::string> CustomVfs::subfiles(const std::string &pathname) const {
    std::vector<std::string> files;
    try {
        std::filesystem::path dirPath(pathname);
        if (!std::filesystem::is_directory(dirPath)) {
            return files;
        }

        for (const auto &entry : std::filesystem::directory_iterator(dirPath)) {
            if (std::filesystem::is_regular_file(entry.status())) {
                files.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << e.what() << std::endl;
    }

    return files;
}

std::string CustomVfs::to_backing(const std::string &pathname) const { return backing_dir + pathname; }
