#ifndef SRC_CUSTOM_VFS_H
#define SRC_CUSTOM_VFS_H

// #include <cerrno>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "directory.h"
#include "file.h"
#include "fuse_wrapper.h"

class CustomVfs : public FuseWrapper {
public:
    explicit CustomVfs(const std::string &string, bool debug = false);

    void init() override;
    void destroy() override;

    void test_files();
    void populate_from_directory(const std::string &path);

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

    // Links
    int symlink(const std::string &target, const std::string &linkpath) override;
    int readlink(const std::string &pathname, char *buf, size_t size) override;
    int link(const std::string &oldpath, const std::string &newpath) override;
    int unlink(const std::string &pathname) override;

    // Directories
    int mkdir(const std::string &pathname, mode_t mode) override;
    int rmdir(const std::string &pathname) override;
    int readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi, readdir_flags flags) override;
    [[nodiscard]] std::vector<std::string> subfiles(const std::string &pathname) const;

private:
    Directory root;
    std::map<std::string, std::shared_ptr<VfsNode>> files{};
};

#endif
