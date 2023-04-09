#include "custom_vfs.h"

#include <filesystem>
#include <fstream>
#include <thread>

CustomVfs::CustomVfs(const std::string &string, bool debug) : root("/", S_IFDIR | (0777 ^ umask)) {
    populate_from_directory(string);

    if (debug) {
        test_files();
    }
}

Directory CustomVfs::root_from_main(int argc, char **argv) {
    std::thread fuse_thread(&CustomVfs::main, this, argc, argv);
    fuse_thread.detach();

    // Fuse-main kills the program, so I run it in a separate thread
    // Could be solved by using low-level fuse API

    return root;
}

void CustomVfs::init() {}

void CustomVfs::destroy() {}

void CustomVfs::populate_from_directory(const std::string &path) {
    for (const auto &entry : std::filesystem::recursive_directory_iterator(path)) {
        std::string filepath = entry.path().string();
        std::string filename = entry.path().filename().string();
        mode_t mode = entry.is_directory() ? S_IFDIR | (0777 ^ umask) : S_IFREG | (0666 ^ umask);

        std::vector<uint8_t> content;

        content.reserve(entry.file_size());
        std::ifstream ifs(filepath, std::ifstream::in | std::ifstream::binary);
        content.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        ifs.close();

        files[filepath] = std::make_shared<File>(filename, mode, content);
    }
}

void CustomVfs::test_files() {
    files["/"] = std::make_shared<Directory>("root", S_IFDIR | (0777 ^ umask));
    files["/helloworld.txt"] = std::make_shared<File>("helloworld.txt", S_IFREG | (0666 ^ umask), "Hello, world.\n");
    files["/dir"] = std::make_shared<Directory>("dir", S_IFDIR | (0777 ^ umask));
    files["/dir/helloworld2.txt"] =
        std::make_shared<File>("helloworld.txt", S_IFREG | (0666 ^ umask), "Another hello, world.\n");
}

std::vector<std::string> CustomVfs::subfiles(const std::string &pathname) const {
    std::vector<std::string> result;
    size_t pathsize = pathname.back() == '/' ? pathname.size() : pathname.size() + 1;

    for (const auto &item : files) {
        const std::string &filepath = item.first;

        const VfsNode &file = *item.second;

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
        const VfsNode &file = *files.at(pathname);
        st->st_mode = file.mode;

        // st->st_size = static_cast<long>(file.content.size());

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
        fill_dir(files.at(file)->name, &st);
    }

    return 0;
}

int CustomVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    const auto node = files.at(pathname);
    if (node->is_directory()) {
        return -EISDIR;
    }
    const auto file = std::static_pointer_cast<File>(node);

    const auto &content = file->content;
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
        files[pathname]->mode = mode;
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

    const auto node = files.at(pathname);
    if (node->is_directory()) {
        return -EISDIR;
    }
    const auto file = std::static_pointer_cast<File>(node);

    auto &content = file->content;
    content.insert(content.begin() + offset, buf, buf + count);

    return static_cast<int>(count);
}

int CustomVfs::truncate(const std::string &pathname, off_t length) {
    if (files.count(pathname)) {
        const auto node = files.at(pathname);

        if (node->is_directory()) {
            return -EISDIR;
        }

        std::static_pointer_cast<File>(node)->content.resize(length);
        return 0;
    } else {
        return -ENOENT;
    }
}

int CustomVfs::mknod(const std::string &pathname, mode_t mode, dev_t dev) {
    files[pathname] = std::make_shared<File>(pathname.substr(pathname.rfind('/') + 1), mode, "");
    return 0;
}

int CustomVfs::mkdir(const std::string &pathname, mode_t mode) {
    files[pathname] = std::make_shared<Directory>(pathname.substr(pathname.rfind('/') + 1), mode);
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

        files[cur_oldpath]->name = cur_newpath.substr(cur_newpath.rfind('/') + 1);
    }

    return 0;
}

int CustomVfs::symlink(const std::string &target, const std::string &linkpath) {
    files[linkpath] = std::make_shared<File>(linkpath.substr(linkpath.rfind('/') + 1), S_IFLNK | 0777, target);
    return 0;
}

int CustomVfs::readlink(const std::string &pathname, char *buf, size_t size) {
    if (files.count(pathname) == 0 || !(files[pathname]->mode & S_IFLNK)) {
        return -ENOENT;
    }

    std::shared_ptr<File> file = std::static_pointer_cast<File>(files[pathname]);
    std::copy(file->content.begin(), file->content.end(), buf);

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

int CustomVfs::utimens(const std::string &pathname, const struct timespec *tv) {
    if (files.count(pathname) == 0) {
        return -ENOENT;
    }
    files[pathname]->times[0] = tv[0];
    files[pathname]->times[1] = tv[1];
    return 0;
}

int CustomVfs::statfs(const std::string &pathname, struct statvfs *stbuf) {
    if (!files.count(pathname)) {
        return -ENOENT;
    }

    memset(stbuf, 0, sizeof(struct statvfs));
    stbuf->f_bsize = 4096;   // Block size
    stbuf->f_frsize = 4096;  // Fragment size

    // Count total and available blocks
    uint64_t total_blocks = 0;
    uint64_t available_blocks = 0;

    for (const auto &item : files) {
        if (item.second->is_directory()) {
            total_blocks += 1;
            available_blocks += 1;
        } else {
            const auto file = std::static_pointer_cast<File>(item.second);
            total_blocks += (file->content.size() + 4095) / 4096;
            available_blocks += (file->content.size() + 4095) / 4096;
        }
    }

    stbuf->f_blocks = total_blocks;
    stbuf->f_bfree = available_blocks;
    stbuf->f_bavail = available_blocks;

    // Count total and available inodes
    stbuf->f_files = static_cast<fsfilcnt_t>(files.size());
    stbuf->f_ffree = 0;
    stbuf->f_favail = 0;

    stbuf->f_fsid = 0;       // File system ID
    stbuf->f_flag = 0;       // Mount flags
    stbuf->f_namemax = 255;  // Maximum filename length

    return 0;
}
