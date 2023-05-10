#include "versioning_vfs.h"

#include <algorithm>

#include "common/config.h"
#include "common/logging.h"
#include "common/prefix_parser.h"

int VersioningVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    if (handle_hook(pathname)) {
        Logging::Debug("Hook handled for %s", pathname.c_str());
        return 0;
    }

    if (PrefixParser::is_prefixed(Path::string_basename(pathname))) {
        return get_wrapped().write(pathname, buf, count, offset, fi);
    }

    int max_version = get_max_version(pathname);

    std::string new_version_path = PrefixParser::apply_prefix(pathname, prefix, {std::to_string(max_version + 1)});
    Logging::Debug("Saving old version of %s to %s", pathname.c_str(), new_version_path.c_str());

    int res = get_wrapped().write(pathname, buf, count, offset, fi);

    if (get_wrapped().copy_file(pathname, new_version_path) == -1) {
        Logging::Error("Failed to store version of %s to %s", pathname.c_str(), new_version_path.c_str());
        return -1;
    }

    return res;
}

int VersioningVfs::get_max_version(const std::string &pathname) {
    int max_version = 0;

    for (const std::string &version_file : get_helper_names(pathname)) {
        std::string version_string = PrefixParser::args_from_prefix(version_file, prefix)[0];
        max_version = std::max(max_version, std::stoi(version_string));
    }

    return max_version;
}

bool VersioningVfs::handle_hook(const std::string &pathname) {
    if (!PrefixParser::contains_prefix(pathname, prefix)) {
        return false;
    }

    auto nonPrefixed = PrefixParser::remove_specific_prefix(pathname, prefix);
    auto args = PrefixParser::args_from_prefix(pathname, prefix);

    if (args.size() == 2) {
        return handle_versioned_command(args[0], args[1], nonPrefixed);
    } else if (args.size() == 1) {
        return handle_non_versioned_command(args[0], nonPrefixed);
    } else {
        Logging::Error("Invalid number of arguments in versioning prefix: %s", pathname.c_str());
        return false;
    }
}

bool VersioningVfs::handle_versioned_command(const std::string &command, const std::string &subArg,
                                             const std::string &arg_path) {
    if (command == "restore") {
        restore_version(arg_path, std::stoi(subArg));
        Logging::Info("Restored version %s of file %s", subArg.c_str(), arg_path.c_str());
        return true;

    } else if (command == "delete") {
        delete_version(arg_path, std::stoi(subArg));
        Logging::Info("Deleted version %s of file %s", subArg.c_str(), arg_path.c_str());
        return true;
    }

    return false;
}

bool VersioningVfs::handle_non_versioned_command(const std::string &command, const std::string &arg_path) {
    if (command == "deleteAll") {
        delete_all_versions(arg_path);
        Logging::Info("Deleted all versions of file %s", arg_path.c_str());
        return true;

    } else if (command == "list") {
        auto versions = get_helper_names(arg_path);

        // write all to pathname
        // auto stream = get_wrapped().get_ifstream(pathname)

        return true;
    }

    return false;
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
    }

    return files;
}

void VersioningVfs::restore_version(const std::string &pathname, int version) {
    std::string restored_path = PrefixParser::apply_prefix(pathname, prefix, {std::to_string(version)});
    get_wrapped().rename(restored_path, pathname, 0);

    Logging::Info("Restored version %d of file %s", version, pathname.c_str());
}

std::vector<std::string> VersioningVfs::get_helper_names(const std::string &pathname) const {
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

std::vector<std::string> VersioningVfs::get_related_files(const std::string &pathname) const {
    Path parent = Path(pathname).parent();
    std::vector<std::string> version_files;

    for (auto &version_file : get_helper_names(pathname)) {
        version_files.push_back((parent / version_file).to_string());
    }

    version_files.push_back(pathname);

    return version_files;
}

void VersioningVfs::delete_all_versions(const std::string &base_name) {
    Path parent = Path(base_name).parent();

    for (const auto &version_file : get_related_files(base_name)) {
        get_wrapped().unlink(version_file);
    }

    get_wrapped().unlink(base_name);
}
