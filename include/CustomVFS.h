#ifndef SRC_CUSTOMVFS_H
#define SRC_CUSTOMVFS_H

#include "fuse_wrapper.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include "fuse_wrapper.h"

class CustomVFS : public FuseWrapper {
   public:

    struct File {
        std::string name;
        mode_t mode{};
        std::string content;

        File(std::string name, mode_t mode, std::string content = "")
            : name(std::move(name)), mode(mode), content(std::move(content)) {}

        File() = default;
        File &operator=(File const &other) = default;
    };

    std::map<std::string, File> files;

    std::vector<std::string> subfiles(std::string const &pathname) {
        std::vector<std::string> result;
        size_t pathsize = pathname[pathname.size() - 1] == '/'
                              ? pathname.size()
                              : pathname.size() + 1;

        for (auto & it : files) {
            std::string const &filepath = it.first;
            File &file = it.second;

            if (file.name.size() + pathsize == filepath.size() &&
                0 == filepath.compare(0, pathname.size(), pathname)) {
                // path in dir
                result.push_back(filepath);
            }
        }
        return result;
    }

    void init() override {
        files.clear();
        files["/"] = File("root", S_IFDIR | (0777 ^ umask));
        files["/helloworld.txt"] = File("helloworld.txt", S_IFREG | (0666 ^ umask), "Hello, world.\n");

        // add directory
        files["/dir"] = File("dir", S_IFDIR | (0777 ^ umask));
        files["/dir/helloworld.txt"] = File("helloworld.txt", S_IFREG | (0666 ^ umask), "Hello, world.\n");
    }

    void destroy() override {}

    /* reading functions */

    int getattr(const std::string &pathname, struct stat *st) override {
        memset(st, 0, sizeof(*st));
        st->st_uid = uid;
        st->st_gid = gid;
        if (files.count(pathname)) {
            File &file = files[pathname];
            st->st_mode = file.mode;
            st->st_size = file.content.size();
            return 0;
        } else {
            return -ENOENT;
        }
    }

    int readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi,
                readdir_flags flags) override {
        struct stat st{};
        (void)off;
        (void)fi;
        (void)flags;

        std::vector<std::string> path_files = subfiles(pathname);

        for (auto & file : path_files) {
            getattr(file, &st);
            fill_dir(this->files[file].name, &st);
        }

        return 0;
    }

    int read(const std::string &pathname, char *buf, size_t count, off_t offset,
             struct fuse_file_info *fi) override {
        std::string &content = files[pathname].content;
        (void)fi;
        if (count + offset > content.size()) {
            if ((size_t)offset > content.size()) {
                count = 0;
            } else {
                count = content.size() - offset;
            }
        }
        memcpy(buf, content.data() + offset, count);
        return static_cast<int>(count);
    }

    /* writing functions */

    int chmod(const std::string &pathname, mode_t mode) override {
        if (files.count(pathname)) {
            files[pathname].mode = mode;
            return 0;
        } else {
            return -ENOENT;
        }
    }

    int write(const std::string &pathname, const char *buf, size_t count,
              off_t offset, struct fuse_file_info *fi) override {
        std::string &content = files[pathname].content;
        size_t precount =
            offset + count > content.size() ? content.size() - offset : count;
        (void)fi;
        content.replace(offset, precount, std::string(buf, buf + count));
        return static_cast<int>(count);
    }

    int truncate(const std::string &pathname, off_t length) override {
        if (files.count(pathname)) {
            files[pathname].content.resize(length);
            return 0;
        } else {
            return -ENOENT;
        }
    }

    int mknod(const std::string &pathname, mode_t mode, dev_t dev) override {
        (void)dev;
        files[pathname] = File(pathname.substr(pathname.rfind('/') + 1), mode);
        return 0;
    }

    int mkdir(const std::string &pathname, mode_t mode) override {
        files[pathname] =
            File(pathname.substr(pathname.rfind('/') + 1), S_IFDIR | mode);
        return 0;
    }

    int unlink(const std::string &pathname) override {
        files.erase(pathname);
        return 0;
    }

    int rmdir(const std::string &pathname) override {
        if (!subfiles(pathname).empty()) {
            return -ENOTEMPTY;
        } else {
            files.erase(pathname);
            return 0;
        }
    }

    int rename(const std::string &oldpath, const std::string &newpath,
               unsigned int flags) override {
        int result = 0;
        size_t idx;

        std::vector<std::string> subfiles = this->subfiles(oldpath);
        for (idx = 0; idx < subfiles.size() && 0 == result; ++idx) {
            result = rename(subfiles[idx],
                            newpath + subfiles[idx].substr(oldpath.size()), flags);
        }
        if (result != 0) {
            while (idx > 0) {
                --idx;
                rename(newpath + subfiles[idx].substr(oldpath.size()), subfiles[idx],
                       flags);
            }
        } else {
            File file = files[oldpath];
            files.erase(oldpath);
            file.name = newpath.substr(newpath.rfind('/') + 1, newpath.size());
            files[newpath] = file;
        }
        return result;
    }
};

#endif  // SRC_CUSTOMVFS_H
