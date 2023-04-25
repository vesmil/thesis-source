#include "versioning_vfs.h"

#include "config.h"

int VersioningVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    int max_version = get_max_version(pathname);

    if (max_version != 0) {
        while (max_version > Config::versioning.stored_versions) {
            std::string oldest_version_path =
                pathname + version_prefix + std::to_string(max_version - Config::versioning.stored_versions);
            wrapped_vfs.unlink(oldest_version_path);

            for (size_t i = max_version - Config::versioning.stored_versions + 1; i <= max_version; i++) {
                std::string old_version_path = pathname + version_prefix + std::to_string(i);
                std::string new_version_path = pathname + version_prefix + std::to_string(i - 1);

                wrapped_vfs.rename(old_version_path, new_version_path, 0);
            }

            max_version--;
        }
    } else {
        max_version = 1;
    }

    std::string new_version_path = pathname + version_prefix + std::to_string(max_version + 1);

    struct stat st {};
    getattr(pathname, &st);

    wrapped_vfs.mknod(new_version_path, st.st_mode, st.st_dev);
    return wrapped_vfs.write(new_version_path, buf, count, offset, fi);
}

int VersioningVfs::get_max_version(const std::string &pathname) {
    auto parent_path = CustomVfs::get_parent(pathname);
    std::vector<std::string> path_files = wrapped_vfs.subfiles(parent_path);

    int max_version = 0;
    for (const std::string &filename : path_files) {
        if (filename.rfind(version_prefix, 0) == 0) {
            int version = std::stoi(filename.substr(version_prefix.size()));
            max_version = std::max(max_version, version);
        }
    }

    return max_version;
}

int VersioningVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    int result;
    if (handle_hook(result, pathname, buf, count, offset, fi)) {
        return result;
    }

    std::string version_path = pathname + version_prefix + std::to_string(get_max_version(pathname));
    return wrapped_vfs.read(version_path, buf, count, offset, fi);
}

bool VersioningVfs::handle_hook(int &result, const std::string &pathname, char *buf, size_t count, off_t offset,
                                struct fuse_file_info *fi) {
    std::string filename = CustomVfs::get_filename(pathname);
    if (filename[0] == '#') {
        std::string command = filename.substr(1, filename.find('#') - 1);
        std::string arg = filename.substr(filename.find('#') + 1);

        // decide what to do based on command
        if (command == "restore") {
            restore_version(pathname, std::stoi(arg));
            result = 0;
            return true;
        }
    }

    return false;
}

std::vector<std::string> VersioningVfs::list_versions(const std::string &pathname) {
    auto parent_path = CustomVfs::get_parent(pathname);
    std::vector<std::string> path_files = wrapped_vfs.subfiles(parent_path);

    std::vector<std::string> version_files;
    for (const std::string &filename : path_files) {
        if (filename.rfind(version_prefix, 0) == 0) {
            version_files.push_back(filename);
        }
    }

    return version_files;
}

void VersioningVfs::delete_version(const std::string &pathname, int version) {
    wrapped_vfs.unlink(pathname + version_prefix + std::to_string(version));
}

int VersioningVfs::readdir(const std::string &pathname, off_t off, struct fuse_file_info *fi,
                           FuseWrapper::readdir_flags flags) {
    return wrapped_vfs.readdir(pathname, off, fi, flags);
}

std::vector<std::string> VersioningVfs::subfiles(const std::string &pathname) const {
    auto files = CustomVfs::subfiles(pathname);

    std::vector<std::string> filtered_files;
    for (const std::string &filename : files) {
        if (filename.rfind(version_prefix + "1") != 0) {
            filtered_files.push_back(filename.substr(0, filename.size() - 2));
        }
    }

    return filtered_files;
}

void VersioningVfs::restore_version(const std::string &pathname, int version) {}
