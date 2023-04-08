#include <utility>

#ifndef SRC_VFS_NODE_H
#define SRC_VFS_NODE_H

struct VfsNode {
    VfsNode(std::string name, mode_t mode) : name(std::move(name)), mode(mode) {}

    std::string name;
    mode_t mode{};
};

#endif  // SRC_VFS_NODE_H
