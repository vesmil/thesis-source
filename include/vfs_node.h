#include <utility>

#ifndef SRC_VFS_NODE_H
#define SRC_VFS_NODE_H

struct VfsNode {
    VfsNode(std::string name, mode_t mode) : name(std::move(name)), mode(mode) {
        clock_gettime(CLOCK_REALTIME, &times[0]);
        clock_gettime(CLOCK_REALTIME, &times[1]);
    }

    timespec times[2]{};

    std::string name;
    mode_t mode{};

    virtual bool is_directory() { return false; }
};

#endif  // SRC_VFS_NODE_H
