#include "CustomVFS.h"

void CustomVFS::init() {
    // TODO
    files.clear();

    files["/"] = File("root", S_IFDIR | (0777 ^ umask));
    files["/helloworld.txt"] =
        File("helloworld.txt", S_IFREG | (0666 ^ umask), "Hello, world.\n");

    // add directory
    files["/dir"] = File("dir", S_IFDIR | (0777 ^ umask));
    files["/dir/helloworld.txt"] =
        File("helloworld.txt", S_IFREG | (0666 ^ umask), "Hello, world.\n");
}

std::vector<std::string> CustomVFS::subfiles(const std::string &pathname) const {
    std::vector<std::string> result;
    size_t pathsize =
        pathname.back() == '/' ? pathname.size() : pathname.size() + 1;

    for (const auto &item : files) {
        const std::string &filepath = item.first;
        const File &file = item.second;

        if (file.name.size() + pathsize == filepath.size() &&
            filepath.compare(0, pathname.size(), pathname) == 0) {
            result.push_back(filepath);
        }
    }
    return result;
}
int CustomVFS::getattr(const std::string &pathname, struct stat *st) {
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
int CustomVFS::readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi,
                       FuseWrapper::readdir_flags flags) {
    struct stat st {};

    std::vector<std::string> path_files = subfiles(pathname);

    for (const auto &file : path_files) {
        getattr(file, &st);
        fill_dir(files.at(file).name, &st);
    }

    return 0;
}
int CustomVFS::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    const std::string &content = files.at(pathname).content;
    if (count + offset > content.size()) {
        if (static_cast<size_t>(offset) > content.size()) {
            count = 0;
        } else {
            count = content.size() - offset;
        }
    }
    memcpy(buf, content.data() + offset, count);
    return static_cast<int>(count);
}
int CustomVFS::chmod(const std::string &pathname, mode_t mode) {
    if (files.count(pathname)) {
        files[pathname].mode = mode;
        return 0;
    } else {
        return -ENOENT;
    }
}
int CustomVFS::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                     struct fuse_file_info *fi) {
    std::string &content = files[pathname].content;
    size_t precount =
        offset + count > content.size() ? content.size() - offset : count;
    content.replace(offset, precount, std::string(buf, buf + count));
    return static_cast<int>(count);
}
int CustomVFS::truncate(const std::string &pathname, off_t length) {
    if (files.count(pathname)) {
        files[pathname].content.resize(length);
        return 0;
    } else {
        return -ENOENT;
    }
}
int CustomVFS::mknod(const std::string &pathname, mode_t mode, dev_t dev) {
    files[pathname] = File(pathname.substr(pathname.rfind('/') + 1), mode);
    return 0;
}
int CustomVFS::mkdir(const std::string &pathname, mode_t mode) {
    files[pathname] =
        File(pathname.substr(pathname.rfind('/') + 1), S_IFDIR | mode);
    return 0;
}
int CustomVFS::unlink(const std::string &pathname) {
    files.erase(pathname);
    return 0;
}
int CustomVFS::rmdir(const std::string &pathname) {
    if (!subfiles(pathname).empty()) {
        return -ENOTEMPTY;
    } else {
        files.erase(pathname);
        return 0;
    }
}
int CustomVFS::rename(const std::string &oldpath, const std::string &newpath, unsigned int flags) {
    if (oldpath == newpath) {
        return 0;
    }

    std::queue<std::pair<std::string, std::string>> path_pairs;
    path_pairs.push({oldpath, newpath});

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
            std::string new_subpath =
                cur_newpath + subfile.substr(cur_oldpath.size());
            path_pairs.push({subfile, new_subpath});
        }

        File file = files[cur_oldpath];
        files.erase(cur_oldpath);
        file.name = cur_newpath.substr(cur_newpath.rfind('/') + 1);
        files[cur_newpath] = file;
    }

    return 0;
}
