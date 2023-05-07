#include "versioning_vfs.h"

#include <algorithm>

#include "common/config.h"
#include "common/logging.h"

int VersioningVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    if (handle_hook(pathname, fi)) {
        Logging::Debug("Hook handled for %s", pathname.c_str());
        return 0;
    }

    int max_version = get_max_version(pathname);
    if (max_version != 0) {
        // Remove oldest versions if needed
        while (max_version > Config::versioning.stored_versions) {
            std::string oldest_version_path =
                pathname + version_suffix + std::to_string(max_version - Config::versioning.stored_versions);
            get_wrapped().unlink(oldest_version_path);

            for (size_t i = max_version - Config::versioning.stored_versions + 1; i <= max_version; i++) {
                std::string old_version_path = pathname + version_suffix + std::to_string(i);
                std::string new_version_path = pathname + version_suffix + std::to_string(i - 1);

                get_wrapped().rename(old_version_path, new_version_path, 0);
            }

            max_version--;
        }
    }

    std::string new_version_path = pathname + version_suffix + std::to_string(max_version + 1);

    if (offset != 0) {
        Logging::Debug("Saving old version of %s to %s", pathname.c_str(), new_version_path.c_str());
        if (get_wrapped().copy_file(pathname, new_version_path) == -1) {
            Logging::Error("Failed to rename %s to %s", pathname.c_str(), new_version_path.c_str());
            return -1;
        }

        Logging::Debug("Writing new version to %s", pathname.c_str());
        return get_wrapped().write(pathname, buf, count, offset, fi);
    }

    struct stat st {};
    getattr(pathname, &st);

    // TODO doesn't work like that - pathname will be empty at this moment
    get_wrapped().rename(pathname, new_version_path, 0);

    get_wrapped().mknod(pathname, st.st_mode, st.st_rdev);
    return get_wrapped().write(pathname, buf, count, offset, fi);
}

int VersioningVfs::unlink(const std::string &pathname) {
    int max_version = get_max_version(pathname);
    auto stored_path = pathname + version_suffix + std::to_string(max_version + 1);

    Logging::Debug("Moved old version of %s to %s", pathname.c_str(), stored_path.c_str());
    return get_wrapped().rename(pathname, stored_path, 0);
}

int VersioningVfs::get_max_version(const std::string &pathname) {
    int max_version = 0;
    for (const std::string &version_file : list_suffixed(pathname)) {
        auto version = std::stoi(version_file.substr(version_file.find(version_suffix) + version_suffix.length()));
        max_version = std::max(max_version, version);
    }

    return max_version;
}

bool VersioningVfs::handle_hook(const std::string &pathname, struct fuse_file_info *fi) {
    std::string filename = Path::string_basename(pathname);

    if (filename[0] != '#') {
        return false;
    }

    auto dashPos = filename.find('-');
    if (dashPos == std::string::npos) {
        return false;
    }

    std::string command = filename.substr(1, dashPos - 1);
    std::string arg = filename.substr(dashPos + 1);

    Path parent = Path(pathname).parent();

    auto underscorePos = command.find('_');

    // TODO write responses
    if (underscorePos != std::string::npos) {
        auto subArg = command.substr(underscorePos + 1);
        command = command.substr(0, underscorePos);

        return handle_versioned_command(command, subArg, (parent / arg).to_string(), pathname, fi);
    }

    return handle_non_versioned_command(command, (parent / arg).to_string(), pathname, fi);
}

bool VersioningVfs::handle_versioned_command(const std::string &command, const std::string &subArg,
                                             const std::string &arg_path, const std::string &pathname,
                                             struct fuse_file_info *fi) {
    // TODO would prob be better if moved to a separate class - map to functions...
    if (command == "restore") {
        restore_version(arg_path, std::stoi(subArg));
        Logging::Info("Restored version %s of file %s\n", subArg.c_str(), arg_path.c_str());
        return true;

    } else if (command == "delete") {
        delete_version(arg_path, std::stoi(subArg));
        Logging::Info("Deleted version %s of file %s\n", subArg.c_str(), arg_path.c_str());
        return true;
    }

    return false;
}

bool VersioningVfs::handle_non_versioned_command(const std::string &command, const std::string &arg_path,
                                                 const std::string &pathname, struct fuse_file_info *fi) {
    if (command == "deleteAll") {
        delete_all_versions(arg_path);
        Logging::Info("Deleted all versions of file %s\n", arg_path.c_str());
        return true;

    } else if (command == "list") {
        auto versions = list_suffixed(arg_path);

        Logging::Info("Listed versions of file %s\n", arg_path.c_str());

        // TODO write to a file
        for (const std::string &version : versions) {
            printf("%s\n", version.c_str());
        }

        return true;
    }

    return false;
}

std::vector<std::string> VersioningVfs::list_suffixed(const std::string &pathname) const {
    std::vector<std::string> path_files = get_wrapped().subfiles(Path::string_parent(pathname));
    std::vector<std::string> version_files;

    for (const std::string &filename : path_files) {
        if (is_version_file(filename)) {
            if (filename.substr(0, filename.find(version_suffix)) == Path::string_basename(pathname)) {
                version_files.push_back(filename);
            }
        }
    }

    return version_files;
}

void VersioningVfs::delete_version(const std::string &pathname, int version) {
    get_wrapped().unlink(pathname + version_suffix + std::to_string(version));
}

int VersioningVfs::fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                            FuseWrapper::fill_dir_flags flags) {
    if (is_version_file(name)) {
        return 0;
    }

    return get_wrapped().fill_dir(name, stbuf, off, flags);
}

bool VersioningVfs::is_version_file(const std::string &pathname) const {
    auto suffix_pos = pathname.rfind(version_suffix);
    if (suffix_pos == std::string::npos) {
        return false;
    } else {
        // Verify if the suffix is proper version number
        std::string version = pathname.substr(suffix_pos + version_suffix.length());
        if (!std::all_of(version.begin(), version.end(), ::isdigit)) return false;

        return true;
    }
}

std::vector<std::string> VersioningVfs::subfiles(const std::string &pathname) const {
    std::vector<std::string> files;

    for (const auto &path : get_wrapped().subfiles(pathname)) {
        if (!is_version_file(path)) {
            files.push_back(path);
        }

        files.push_back(path);
    }
    return files;
}

void VersioningVfs::restore_version(const std::string &pathname, int version) {
    int max_version = get_max_version(pathname);

    get_wrapped().rename(pathname, pathname + version_suffix + std::to_string(max_version + 1), 0);
    get_wrapped().rename(pathname + version_suffix + std::to_string(version), pathname, 0);

    Logging::Info("Restored version %d of file %s", version, pathname.c_str());
}

std::vector<std::string> VersioningVfs::get_related_files(const std::string &pathname) const {
    auto version_files = list_suffixed(pathname);
    version_files.push_back(pathname);
    return version_files;
}

void VersioningVfs::delete_all_versions(const std::string &base_name) {
    Path parent = Path(base_name).parent();
    for (const auto &version_file : list_suffixed(base_name)) {
        get_wrapped().unlink((parent / version_file).to_string());
    }
}
