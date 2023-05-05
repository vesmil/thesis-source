#include "versioning_vfs.h"

#include <algorithm>

#include "config.h"
#include "logging.h"

int VersioningVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    // Hook is in format #command_subArg-path
    if (handle_hook(pathname, fi)) {
        Log::Debug("Hook handled for %s", pathname.c_str());
        return 0;
    }

    return CustomVfs::read(pathname, buf, count, offset, fi);
}

int VersioningVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    int max_version = get_max_version(pathname);

    if (max_version != 0) {
        // Remove oldest versions if needed
        while (max_version > Config::versioning.stored_versions) {
            std::string oldest_version_path =
                pathname + version_suffix + std::to_string(max_version - Config::versioning.stored_versions);
            CustomVfs::unlink(oldest_version_path);

            for (size_t i = max_version - Config::versioning.stored_versions + 1; i <= max_version; i++) {
                std::string old_version_path = pathname + version_suffix + std::to_string(i);
                std::string new_version_path = pathname + version_suffix + std::to_string(i - 1);

                CustomVfs::rename(old_version_path, new_version_path, 0);
            }

            max_version--;
        }
    }

    std::string new_version_path = pathname + version_suffix + std::to_string(max_version + 1);

    struct stat st {};
    getattr(pathname, &st);

    if (CustomVfs::rename(pathname, new_version_path, 0) == -1) {
        Log::Error("Failed to rename %s to %s", pathname.c_str(), new_version_path.c_str());
        return -1;
    }

    Log::Debug("Saved old version of %s to %s", pathname.c_str(), new_version_path.c_str());

    CustomVfs::mknod(pathname, st.st_mode, st.st_dev);

    Log::Debug("Writing new version to %s", pathname.c_str());
    return CustomVfs::write(pathname, buf, count, offset, fi);
}

int VersioningVfs::unlink(const std::string &pathname) {
    int max_version = get_max_version(pathname);
    auto stored_path = pathname + version_suffix + std::to_string(max_version + 1);

    CustomVfs::rename(pathname, stored_path, 0);
    Log::Debug("Saved old version of %s to %s", pathname.c_str(), stored_path.c_str());

    return CustomVfs::unlink(pathname);
}

int VersioningVfs::get_max_version(const std::string &pathname) {
    auto parent_path = CustomVfs::get_parent(pathname);
    std::vector<std::string> path_files = CustomVfs::subfiles(parent_path);

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
        return true;

    } else if (command == "delete") {
        delete_version(arg, std::stoi(subArg));
        printf("Deleted version %s of file %s\n", subArg.c_str(), arg.c_str());
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

        // TODO write result to pathname
        for (const std::string &version : versions) {
            printf("%s\n", version.c_str());
        }

        return true;
    }

    return false;
}

std::vector<std::string> VersioningVfs::list_versions(const std::string &pathname) const {
    // TODO
    std::vector<std::string> path_files = CustomVfs::subfiles(CustomVfs::get_parent(pathname));

    std::vector<std::string> version_files;

    for (const std::string &filename : path_files) {
        if (filename.substr(0, filename.find(version_suffix)) == pathname) {
            if (filename.length() == pathname.length()) {
                continue;
            }

            size_t pos = filename.find_last_of(version_suffix);
            int version = std::stoi(filename.substr(pos + 1));
            version_files.push_back(std::to_string(version));
        }
    }

    return version_files;
}

void VersioningVfs::delete_version(const std::string &pathname, int version) {
    CustomVfs::unlink(pathname + version_suffix + std::to_string(version));
}

int VersioningVfs::fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                            FuseWrapper::fill_dir_flags flags) {
    if (is_version_file(name)) {
        return 0;
    }

    return CustomVfs::fill_dir(name, stbuf, off, flags);
}

bool VersioningVfs::is_version_file(const std::string &pathname) {
    auto suffix_pos = pathname.rfind(version_suffix);
    if (suffix_pos == std::string::npos) {
        return false;
    } else {
        // Verify if the suffix is a version number
        std::string version = pathname.substr(suffix_pos + version_suffix.length());
        if (!std::all_of(version.begin(), version.end(), ::isdigit)) return false;

        return true;
    }
}

std::vector<std::string> VersioningVfs::subfiles(const std::string &pathname) const {
    std::vector<std::string> files;

    for (const auto &path : CustomVfs::subfiles(pathname)) {
        if (path.rfind(version_suffix) == std::string::npos) {
            files.push_back(path);
        }

        files.push_back(path);
    }

    return files;
}

void VersioningVfs::restore_version(const std::string &pathname, int version) {
    int max_version = get_max_version(pathname);
    CustomVfs::rename(pathname, pathname + version_suffix + std::to_string(max_version + 1), 0);

    CustomVfs::rename(pathname + version_suffix + std::to_string(version), pathname, 0);

    Log::Info("Restored version %d of file %s", version, pathname.c_str());
}

std::vector<std::string> VersioningVfs::get_related_files(const std::string &pathname) const {
    auto version_files = list_versions(pathname);
    version_files.push_back(pathname);
    return version_files;
}