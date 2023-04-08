#ifndef SRC_FILE_H
#define SRC_FILE_H

#include <utility>

#include "vfs_node.h"

struct File : public VfsNode {
    std::vector<uint8_t> content;
    timespec times[2] = {};

    File(const std::string &name, mode_t mode, std::vector<uint8_t> content = {})
        : VfsNode(name, mode), content(std::move(content)) {}
    File(const std::string &name, mode_t mode, std::string content)
        : VfsNode(name, mode), content(content.begin(), content.end()) {}

    // TODO Overridden methods for file using CustomVfs
};

#endif  // SRC_FILE_H
