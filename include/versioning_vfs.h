#ifndef SRC_VERSIONING_VFS_H
#define SRC_VERSIONING_VFS_H

#include "vfs_decorator.h"

class VersioningVfs : public VfsDecorator {
public:
    explicit VersioningVfs(CustomVfs &wrapped_vfs) : VfsDecorator(wrapped_vfs) {}

    int write(const std::string &pathname, const char *buf, size_t count, off_t offset,
              struct fuse_file_info *fi) override {
        // Read the current file content
        struct stat st {};
        wrapped_vfs_.getattr(pathname, &st);
        off_t current_size = st.st_size;

        char *current_content = new char[current_size];
        wrapped_vfs_.read(pathname, current_content, current_size, 0, fi);

        // Check for .v_ files to see if there are any versions
        std::vector<std::string> path_files = wrapped_vfs_.subfiles(pathname);
        int max_version = 0;
        for (const std::string &filename : path_files) {
            if (filename.rfind(".v_", 0) == 0) {
                int version = std::stoi(filename.substr(3));
                max_version = std::max(max_version, version);
            }
        }

        // Store the current content as a new version
        std::string new_version_path = pathname + "/.v_" + std::to_string(max_version + 1);
        wrapped_vfs_.mknod(new_version_path, st.st_mode, st.st_dev);
        wrapped_vfs_.write(new_version_path, current_content, current_size, 0, fi);

        delete[] current_content;

        // Delegate the write operation to the wrapped VFS
        return wrapped_vfs_.write(pathname, buf, count, offset, fi);
    }
};

#endif  // SRC_VERSIONING_VFS_H
