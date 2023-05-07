#include "versioning_vfs.h"

#include <algorithm>

#include "common/config.h"
#include "common/logging.h"
#include "common/prefix_parser.h"

int VersioningVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    if (handle_hook(pathname, fi)) {
        Logging::Debug("Hook handled for %s", pathname.c_str());
        return 0;
    }

    int max_version = get_max_version(pathname);
    std::string new_version_path = PrefixParser::apply_prefix(pathname, prefix, {std::to_string(max_version + 1)});

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
    std::string stored_path = PrefixParser::apply_prefix(pathname, prefix, {std::to_string(max_version + 1)});

    Logging::Debug("Moved old version of %s to %s", pathname.c_str(), stored_path.c_str());
    return get_wrapped().rename(pathname, stored_path, 0);
}

int VersioningVfs::get_max_version(const std::string &pathname) {
    int max_version = 0;

    for (const std::string &version_file : list_helper_files(pathname)) {
        std::string version_string = PrefixParser::args_from_prefix(version_file, prefix)[0];
        max_version = std::max(max_version, std::stoi(version_string));
    }

    return max_version;
}

bool VersioningVfs::handle_hook(const std::string &pathname, struct fuse_file_info *fi) {
    std::string filename = Path::string_basename(pathname);

    // TODO should also use prefix_parser

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
        auto versions = list_helper_files(arg_path);

        Logging::Info("Listed versions of file %s\n", arg_path.c_str());

        // TODO write to a file
        for (const std::string &version : versions) {
            printf("%s\n", version.c_str());
        }

        return true;
    }

    return false;
}

std::vector<std::string> VersioningVfs::list_helper_files(const std::string &pathname) const {
    std::vector<std::string> path_files = get_wrapped().subfiles(Path::string_parent(pathname));
    std::vector<std::string> version_files;

    for (const std::string &filename : path_files) {
        if (is_version_file(filename)) {
            if (Path::string_basename(PrefixParser::get_nonprefixed(filename)) == Path::string_basename(pathname)) {
                version_files.push_back(filename);
            }
        }
    }

    return version_files;
}

void VersioningVfs::delete_version(const std::string &pathname, int version) {
    std::string version_path = PrefixParser::apply_prefix(pathname, prefix, {std::to_string(version)});
    get_wrapped().unlink(version_path);
}

int VersioningVfs::fill_dir(const std::string &name, const struct stat *stbuf, off_t off,
                            FuseWrapper::fill_dir_flags flags) {
    if (is_version_file(name)) {
        return 0;
    }

    return get_wrapped().fill_dir(name, stbuf, off, flags);
}

bool VersioningVfs::is_version_file(const std::string &pathname) const {
    std::vector<std::string> parsed = PrefixParser::args_from_prefix(pathname, prefix);
    return parsed.size() == 1 && std::all_of(parsed[0].begin(), parsed[0].end(), ::isdigit);
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

    std::string new_path = PrefixParser::apply_prefix(pathname, prefix, {std::to_string(max_version + 1)});
    get_wrapped().rename(pathname, new_path, 0);

    std::string restored_path = PrefixParser::apply_prefix(pathname, prefix, {std::to_string(version)});
    get_wrapped().rename(restored_path, pathname, 0);

    Logging::Info("Restored version %d of file %s", version, pathname.c_str());
}

std::vector<std::string> VersioningVfs::get_related_files(const std::string &pathname) const {
    auto version_files = list_helper_files(pathname);
    version_files.push_back(pathname);

    return version_files;
}

void VersioningVfs::delete_all_versions(const std::string &base_name) {
    Path parent = Path(base_name).parent();
    for (const auto &version_file : list_helper_files(base_name)) {
        get_wrapped().unlink((parent / version_file).to_string());
    }
}
