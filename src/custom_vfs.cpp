#include "custom_vfs.h"

#include <filesystem>

CustomVfs::CustomVfs(const std::string &string) { populate_from_directory(string); }

void CustomVfs::init() {}

void CustomVfs::populate_from_directory(const std::string &path) {
    for (const auto &entry : std::filesystem::recursive_directory_iterator(path)) {
        std::string filepath = entry.path().string();
        std::string filename = entry.path().filename().string();
        mode_t mode = entry.is_directory() ? S_IFDIR | (0777 ^ umask) : S_IFREG | (0666 ^ umask);
        std::string content = entry.is_regular_file()
                                  ? std::filesystem::file_size(entry) > 0
                                        ? std::filesystem::file_size(entry) > 1000000  ? "File too large to display"
                                          : std::filesystem::file_size(entry) > 100000 ? "File too large to display"
                                          : std::filesystem::file_size(entry) > 10000  ? "File too large to display"
                                          : std::filesystem::file_size(entry) > 1000   ? "File too large to display"
                                          : std::filesystem::file_size(entry) > 100    ? "File too large to display"
                                          : std::filesystem::file_size(entry) > 10     ? "File too large to display"
                                          : std::filesystem::file_size(entry) > 1      ? "File too large to display"
                                          : std::filesystem::file_size(entry) > 0      ? "File too large to display"
                                                                                       : ""
                                        : ""
                                  : "";

        files[filepath] = File(filename, mode, content);
    }
}

void CustomVfs::test_files() {
    if (files.empty()) {
        files["/"] = File("root", S_IFDIR | (0777 ^ umask));
        files["/helloworld.txt"] = File("helloworld.txt", S_IFREG | (0666 ^ umask), "Hello, world.\n");

        // add directory
        files["/dir"] = File("dir", S_IFDIR | (0777 ^ umask));
        files["/dir/helloworld.txt"] = File("helloworld.txt", S_IFREG | (0666 ^ umask), "Hello, world.\n");
    }

    std::string log = "Files in VFS:\n";

    for (const auto &item : files) {
        const std::string &filepath = item.first;
        const File &file = item.second;
        log += filepath + "\n";
    }

    files["/log.txt"] = File("log.txt", S_IFREG | (0666 ^ umask), log);
}

std::vector<std::string> CustomVfs::subfiles(const std::string &pathname) const {
    std::vector<std::string> result;
    size_t pathsize = pathname.back() == '/' ? pathname.size() : pathname.size() + 1;

    for (const auto &item : files) {
        const std::string &filepath = item.first;
        const File &file = item.second;

        if (file.name.size() + pathsize == filepath.size() && filepath.compare(0, pathname.size(), pathname) == 0) {
            result.push_back(filepath);
        }
    }
    return result;
}

int CustomVfs::getattr(const std::string &pathname, struct stat *st) {
    memset(st, 0, sizeof(*st));
    st->st_uid = uid;
    st->st_gid = gid;

    if (files.count(pathname)) {
        const File &file = files.at(pathname);
        st->st_mode = file.mode;
        st->st_size = static_cast<long>(file.content.size());
        return 0;
    } else {
        return -ENOENT;
    }
}
int CustomVfs::readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi,
                       FuseWrapper::readdir_flags flags) {
    struct stat st {};

    std::vector<std::string> path_files = subfiles(pathname);

    for (const auto &file : path_files) {
        getattr(file, &st);
        fill_dir(files.at(file).name, &st);
    }

    return 0;
}
int CustomVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    const std::string &content = files.at(pathname).content;
    if (count + offset > content.size()) {
        if (static_cast<size_t>(offset) > content.size()) {
            count = 0;
        } else {
            count = content.size() - offset;
        }
    }

    std::copy(content.data() + offset, content.data() + offset + count, buf);
    return static_cast<int>(count);
}

int CustomVfs::chmod(const std::string &pathname, mode_t mode) {
    if (files.count(pathname)) {
        files[pathname].mode = mode;
        return 0;
    } else {
        return -ENOENT;
    }
}

int CustomVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                     struct fuse_file_info *fi) {
    if (files.count(pathname) == 0) {
        return -ENOENT;
    }
    std::string &content = files[pathname].content;
    size_t precount = offset + count > content.size() ? content.size() - offset : count;
    content.replace(offset, precount, std::string(buf, buf + count));
    return static_cast<int>(count);
}

int CustomVfs::truncate(const std::string &pathname, off_t length) {
    if (files.count(pathname)) {
        files[pathname].content.resize(length);
        return 0;
    } else {
        return -ENOENT;
    }
}

int CustomVfs::mknod(const std::string &pathname, mode_t mode, dev_t dev) {
    files[pathname] = File(pathname.substr(pathname.rfind('/') + 1), mode);
    return 0;
}

int CustomVfs::mkdir(const std::string &pathname, mode_t mode) {
    files[pathname] = File(pathname.substr(pathname.rfind('/') + 1), S_IFDIR | mode);
    return 0;
}

int CustomVfs::unlink(const std::string &pathname) {
    files.erase(pathname);
    return 0;
}

int CustomVfs::rmdir(const std::string &pathname) {
    if (!subfiles(pathname).empty()) {
        return -ENOTEMPTY;
    } else {
        files.erase(pathname);
        return 0;
    }
}

int CustomVfs::rename(const std::string &oldpath, const std::string &newpath, unsigned int flags) {
    if (oldpath == newpath) {
        return 0;
    }

    std::queue<std::pair<std::string, std::string>> path_pairs;
    path_pairs.emplace(oldpath, newpath);

    while (!path_pairs.empty()) {
        std::string cur_oldpath = path_pairs.front().first;
        std::string cur_newpath = path_pairs.front().second;
        path_pairs.pop();

        if (!files.count(cur_oldpath)) {
            return -ENOENT;
        }

        if (files.count(cur_newpath)) {
            return -EEXIST;
        }

        std::vector<std::string> subfiles_list = subfiles(cur_oldpath);
        for (const auto &subfile : subfiles_list) {
            std::string new_subpath = cur_newpath + subfile.substr(cur_oldpath.size());
            path_pairs.emplace(subfile, new_subpath);
        }

        File file = files[cur_oldpath];
        files.erase(cur_oldpath);
        file.name = cur_newpath.substr(cur_newpath.rfind('/') + 1);
        files[cur_newpath] = file;
    }

    return 0;
}

int CustomVfs::symlink(const std::string &target, const std::string &linkpath) {
    files[linkpath] = File(linkpath.substr(linkpath.rfind('/') + 1), S_IFLNK | 0777, target);
    return 0;
}

int CustomVfs::readlink(const std::string &pathname, char *buf, size_t size) {
    if (files.count(pathname) == 0 || !(files[pathname].mode & S_IFLNK)) {
        return -ENOENT;
    }
    strncpy(buf, files[pathname].content.c_str(), size);
    return 0;
}

int CustomVfs::link(const std::string &oldpath, const std::string &newpath) {
    if (files.count(oldpath) == 0) {
        return -ENOENT;
    }
    files[newpath] = files[oldpath];
    return 0;
}

int CustomVfs::open(const std::string &pathname, struct fuse_file_info *fi) {
    if (files.count(pathname) == 0) {
        return -ENOENT;
    }
    return 0;
}

int CustomVfs::release(const std::string &pathname, struct fuse_file_info *fi) {
    if (files.count(pathname) == 0) {
        return -ENOENT;
    }
    return 0;
}
