#include "versioning_vfs.h"

#include <filesystem>

#include "config.h"

int VersioningVfs::getattr(const std::string &pathname, struct stat *st) {
    int max_version = get_max_version(pathname);
    
    // TODO get current version and not max

    if (max_version == 0) {
        return wrapped_vfs.getattr(pathname, st);
    }

    return CustomVfs::getattr(pathname + version_suffix + std::to_string(max_version), st);
}

int VersioningVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    // Hook is in format #command_subArg-path
    if (handle_hook(pathname, fi)) {
        return 0;
    }

    int max_version = get_max_version(pathname);
    if (max_version == 0) {
        return wrapped_vfs.read(pathname, buf, count, offset, fi);
    }

    std::string version_path = pathname + version_suffix + std::to_string(max_version);
    return wrapped_vfs.read(version_path, buf, count, offset, fi);
}

int VersioningVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    int max_version = get_max_version(pathname);

    if (max_version != 0) {
        // Remove oldest versions if needed
        while (max_version > Config::versioning.stored_versions) {
            std::string oldest_version_path =
                pathname + version_suffix + std::to_string(max_version - Config::versioning.stored_versions);
            wrapped_vfs.unlink(oldest_version_path);

            for (size_t i = max_version - Config::versioning.stored_versions + 1; i <= max_version; i++) {
                std::string old_version_path = pathname + version_suffix + std::to_string(i);
                std::string new_version_path = pathname + version_suffix + std::to_string(i - 1);

                wrapped_vfs.rename(old_version_path, new_version_path, 0);
            }

            max_version--;
        }
    }

    std::string new_version_path = pathname + version_suffix + std::to_string(max_version + 1);

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
        if (parent_file.substr(0, parent_file.find(version_suffix)) == pathname) {
            if (parent_file.length() == pathname.length()) {
                continue;
            }

            size_t pos = parent_file.find_last_of(version_suffix);
            int version = std::stoi(parent_file.substr(pos + 1));
            max_version = std::max(max_version, version);
        }
    }

    return max_version;
}

bool VersioningVfs::handle_hook(const std::string &pathname, struct fuse_file_info *fi) {
    std::string filename = CustomVfs::get_filename(pathname);

    if (filename[0] != '#') {
        return false;
    }

    auto dashPos = filename.find('-');
    if (dashPos == std::string::npos) {
        return false;
    }

    std::string command = filename.substr(1, dashPos - 1);
    std::string arg = filename.substr(dashPos + 1);

    auto underscorePos = command.find('_');
    if (underscorePos != std::string::npos) {
        auto subArg = command.substr(underscorePos + 1);
        command = command.substr(0, underscorePos);

        return handle_versioned_command(command, subArg, arg, pathname, fi);
    }

    return handle_non_versioned_command(command, arg, pathname, fi);
}

bool VersioningVfs::handle_versioned_command(const std::string &command, const std::string &subArg,
                                             const std::string &arg, const std::string &pathname,
                                             struct fuse_file_info *fi) {
    if (command == "restore") {
        restore_version(arg, std::stoi(subArg));
        printf("Restored version %s of file %s\n", subArg.c_str(), arg.c_str());

        // TODO write result to pathname

        return true;

    } else if (command == "delete") {
        delete_version(arg, std::stoi(subArg));
        printf("Deleted version %s of file %s\n", subArg.c_str(), arg.c_str());

        // TODO write result to pathname

        return true;
    }

    return false;
}

bool VersioningVfs::handle_non_versioned_command(const std::string &command, const std::string &arg,
                                                 const std::string &pathname, struct fuse_file_info *fi) {
    if (command == "deleteAll") {
        // delete all versions
        printf("Deleted all versions of file %s\n", arg.c_str());

    } else if (command == "list") {
        auto versions = list_versions(arg);
        printf("Listed versions of file %s\n", arg.c_str());

        for (const std::string &version : versions) {
            printf("%s\n", version.c_str());
        }

        return true;
    }

    return false;
}

std::vector<std::string> VersioningVfs::list_versions(const std::string &pathname) {
    auto parent_path = CustomVfs::get_parent(pathname);
    std::vector<std::string> path_files = wrapped_vfs.subfiles(parent_path);

    std::vector<std::string> version_files;
    for (const std::string &filename : path_files) {
        if (filename.rfind(version_suffix, 0) == 0) {
            version_files.push_back(filename);
        }
    }

    return version_files;
}

void VersioningVfs::delete_version(const std::string &pathname, int version) {
    wrapped_vfs.unlink(pathname + version_suffix + std::to_string(version));
}

int VersioningVfs::fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                            FuseWrapper::fill_dir_flags flags) {
    if (name.rfind(version_suffix) == std::string::npos) {
        return CustomVfs::fill_dir(name, stbuf, off, flags);
    } else {
        return 0;
    }
}

std::vector<std::string> VersioningVfs::subfiles(const std::string &pathname) const {
    std::vector<std::string> files;

    for (const auto &entry : std::filesystem::directory_iterator(pathname)) {
        std::string path = entry.path();

        if (path.rfind(version_suffix) == 0) {
            files.push_back(path);
        }

        files.push_back(path);
    }

    return files;
}

void VersioningVfs::restore_version(const std::string &pathname, int version) {}
