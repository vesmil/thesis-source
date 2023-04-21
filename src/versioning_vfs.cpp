#include "versioning_vfs.h"
int VersioningVfs::write(const std::string &pathname, const char *buf, size_t count, off_t offset,
                         struct fuse_file_info *fi) {
    // Read the current file content
    struct stat st {};
    wrapped_vfs.getattr(pathname, &st);
    off_t current_size = st.st_size;

    char *current_content = new char[current_size];
    wrapped_vfs.read(pathname, current_content, current_size, 0, fi);

    // Check for .v_ files to see if there are any versions
    std::vector<std::string> path_files = wrapped_vfs.subfiles(pathname);
    int max_version = 0;
    for (const std::string &filename : path_files) {
        if (filename.rfind(".v_", 0) == 0) {
            int version = std::stoi(filename.substr(3));
            max_version = std::max(max_version, version);
        }
    }

    // TODO also configure how many versions to keep...

    // TODO diff based approach

    // Store the current content as a new version
    std::string new_version_path = pathname + "/.v_" + std::to_string(max_version + 1);
    wrapped_vfs.mknod(new_version_path, st.st_mode, st.st_dev);
    wrapped_vfs.write(new_version_path, current_content, current_size, 0, fi);

    delete[] current_content;

    // Delegate the write operation to the wrapped VFS
    return wrapped_vfs.write(pathname, buf, count, offset, fi);
}

int VersioningVfs::read(const std::string &pathname, char *buf, size_t count, off_t offset, struct fuse_file_info *fi) {
    int result;
    if (handle_hook(result, pathname, buf, count, offset, fi)) {
        return result;
    }

    return CustomVfs::read(pathname, buf, count, offset, fi);
}

bool VersioningVfs::handle_hook(int &result, const std::string &pathname, char *buf, size_t count, off_t offset,
                                struct fuse_file_info *fi) {
    if (pathname[0] == '#') {
        // save_version

        // list_versions

        return true;
    }

    return false;
}

std::vector<std::string> VersioningVfs::list_versions(const std::string &pathname) {
    // returns a list of version files... - not sure if should be strings yet

    return {};
}

void VersioningVfs::restore_version(const std::string &pathname, int version) {
    // set as current version?
}

void VersioningVfs::delete_version(const std::string &pathname, int version) {
    wrapped_vfs.unlink(pathname + "/.v_" + std::to_string(version));
}