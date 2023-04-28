#include "versioning_vfs.h"

#include <filesystem>

#include "config.h"

int VersioningVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    int result;
 
    if (handle_hook(result, pathname, buf, count, offset, fi)) {
        return result;
    }

    int max_version = get_max_version(pathname);

    // This is just in case there were no writes to the file
    if (max_version == 0) {
        return wrapped_vfs.read(pathname, buf, count, offset, fi);
    }

    std::string version_path = pathname + version_prefix + std::to_string(max_version);
    return wrapped_vfs.read(version_path, buf, count, offset, fi);
}

int VersioningVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    int max_version = get_max_version(pathname);

    if (max_version != 0) {
        // Remove oldest versions if needed
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
    }

    std::string new_version_path = pathname + version_prefix + std::to_string(max_version + 1);

    struct stat st {};
    getattr(pathname, &st);

    wrapped_vfs.mknod(new_version_path, st.st_mode, st.st_dev);
    return wrapped_vfs.write(new_version_path, buf, count, offset, fi);
}

int VersioningVfs::unlink(const std::string &pathname) {
    // TODO Will not delete versions but will hide the file
    return CustomVfs::unlink(pathname);
}

int VersioningVfs::get_max_version(const std::string &pathname) {
    auto parent_path = CustomVfs::get_parent(pathname);
    std::vector<std::string> path_files = wrapped_vfs.subfiles(parent_path);

    int max_version = 0;
    for (const std::string &parent_file : path_files) {
        if (parent_file.rfind(pathname) != std::string::npos) {
            if (parent_file.rfind(version_prefix) != std::string::npos) {
                size_t pos = parent_file.find_last_of(version_prefix);
                int version = std::stoi(parent_file.substr(pos + 1));
                max_version = std::max(max_version, version);
            }
        }
    }

    return max_version;
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

        // TODO ...
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
    return CustomVfs::readdir(pathname, off, fi, flags);
}

int VersioningVfs::fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                            FuseWrapper::fill_dir_flags flags) {
    if (name.rfind(version_prefix) == std::string::npos) {
        return CustomVfs::fill_dir(name, stbuf, off, flags);
    } else {
        return 0;
    }
}

std::vector<std::string> VersioningVfs::subfiles(const std::string &pathname) const {
    std::vector<std::string> files;

    for (const auto &entry : std::filesystem::directory_iterator(pathname)) {
        std::string path = entry.path();

        if (path.rfind(version_prefix) == 0) {
            files.push_back(path);
        }

        files.push_back(path);
    }

    return files;
}

void VersioningVfs::restore_version(const std::string &pathname, int version) {}
