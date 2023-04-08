#ifndef SRC_DIRECTORY_H
#define SRC_DIRECTORY_H

#include "file.h"
#include "vfs_node.h"

struct Directory : public VfsNode {
    std::map<std::string, File> files;
    std::map<std::string, Directory> subdirectories;

    Directory(const std::string &name, mode_t mode) : VfsNode(name, mode) {}
    Directory &operator=(Directory const &other) = default;

    bool is_directory() override { return true; }

    // TODO Overridden methods for directory using CustomVfs
};

#endif  // SRC_DIRECTORY_H
