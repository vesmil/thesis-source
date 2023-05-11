#include "versioning_vfs.h"

#include <time.h>

#include <algorithm>
#include <fstream>

#include "common/config.h"
#include "common/logging.h"
#include "common/prefix_parser.h"

int VersioningVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    try {
        if (handle_hook(pathname)) {
            Logging::Debug("Hook handled for %s", pathname.c_str());
            return 0;
        }
    } catch (std::exception &e) {
        Logging::Error("Failed to handle hook for %s: %s", pathname.c_str(), e.what());
        return -1;
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

    for (const std::string &version_file : get_related_names(pathname)) {
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
        return handle_versioned_command(args[0], args[1], nonPrefixed, pathname);
    } else if (args.size() == 1) {
        return handle_non_versioned_command(args[0], nonPrefixed, pathname);
    } else {
        Logging::Error("Invalid number of arguments in versioning prefix: %s", pathname.c_str());
        return false;
    }
}

bool VersioningVfs::handle_versioned_command(const std::string &command, const std::string &subArg,
                                             const std::string &arg_path,
                                             [[maybe_unused]] const std::string &hook_file) {
    if (command == "restore") {
        if (!get_wrapped().exists(PrefixParser::apply_prefix(arg_path, prefix, {subArg}))) {
            auto stream = CustomVfs::get_ofstream(hook_file, std::ios::binary);
            *stream << "Requested file or version not available!" << std::endl;
            stream->close();
            return true;
        }

        restore_version(arg_path, std::stoi(subArg));
        Logging::Info("Restored version %s of file %s", subArg.c_str(), arg_path.c_str());
        return true;

    } else if (command == "delete") {
        if (!get_wrapped().exists(PrefixParser::apply_prefix(arg_path, prefix, {subArg}))) {
            auto stream = CustomVfs::get_ofstream(hook_file, std::ios::binary);
            *stream << "Requested file or version not available!" << std::endl;
            stream->close();
            return true;
        }

        delete_version(arg_path, std::stoi(subArg));
        Logging::Info("Deleted version %s of file %s", subArg.c_str(), arg_path.c_str());
        return true;
    }

    return false;
}

bool VersioningVfs::handle_non_versioned_command(const std::string &command, const std::string &arg_path,
                                                 const std::string &hook_file) {
    if (command == "deleteAll") {
        if (!get_wrapped().exists(arg_path)) {
            auto stream = CustomVfs::get_ofstream(hook_file, std::ios::binary);
            *stream << "Requested file not available! Deleting all failed." << std::endl;
            stream->close();
            return true;
        }

        delete_all_versions(arg_path);
        Logging::Info("Deleted all versions of file %s", arg_path.c_str());
        return true;

    } else if (command == "list") {
        list_versions(arg_path, hook_file);
        return true;
    }

    return false;
}

void VersioningVfs::list_versions(const std::string &arg_path, const std::string &hook_file) {
    if (!get_wrapped().exists(arg_path)) {
        auto stream = CustomVfs::get_ofstream(hook_file, std::ios::binary);
        *stream << "Requested file not available! Listing failed." << std::endl;
        stream->close();
        return;
    }

    auto versions = get_related_names(arg_path);
    auto stream = CustomVfs::get_ofstream(hook_file, std::ios::binary);

    std::vector<int> version_numbers;
    version_numbers.reserve(versions.size());

    for (const auto &version : versions) {
        version_numbers.push_back(std::stoi(PrefixParser::args_from_prefix(version, prefix)[0]));
    }

    std::vector<std::string> version_times;
    version_times.reserve(versions.size());
    Path parent = Path(arg_path).parent();

    for (const auto &version : versions) {
        auto stbuf = std::make_unique<struct stat>();
        get_wrapped().getattr(parent / version, stbuf.get());

        const auto rawtime = stbuf->st_mtime;
        const auto timeinfo = localtime(&rawtime);

        std::ostringstream oss;
        oss << std::put_time(timeinfo, "%Y-%m-%d %H:%M");

        version_times.push_back(oss.str());
    }

    std::sort(version_numbers.begin(), version_numbers.end());
    for (int i = 0; i < version_numbers.size(); i++) {
        *stream << version_numbers[i] << " - " << version_times[i] << "\n";
    }

    stream->close();
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

    if (get_wrapped().exists(pathname)) {
        get_wrapped().unlink(pathname);
    }

    get_wrapped().copy_file(restored_path, pathname);

    Logging::Info("Restored version %d of file %s", version, pathname.c_str());
}

std::vector<std::string> VersioningVfs::get_related_names(const std::string &pathname) const {
    std::vector<std::string> path_files = get_wrapped().subfiles(Path::string_parent(pathname));
    std::vector<std::string> version_files;

    for (const std::string &filename : path_files) {
        if (is_version_file(filename)) {
            if (Path::string_basename(Path::string_basename(PrefixParser::get_nonprefixed(filename))) ==
                Path::string_basename(pathname)) {
                version_files.push_back(filename);
            }
        }
    }

    return version_files;
}

std::vector<std::string> VersioningVfs::get_related_files(const std::string &pathname) const {
    Path parent = Path(pathname).parent();
    std::vector<std::string> version_files;

    for (auto &version_file : get_related_names(pathname)) {
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
