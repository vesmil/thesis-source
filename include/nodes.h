#ifndef SRC_VFS_NODE_H
#define SRC_VFS_NODE_H

#include <cstdlib>
#include <ctime>
#include <map>
#include <string>
#include <utility>
#include <vector>

// Mostly used for debug purposes
// Was used to experiment with VFS based on nodes

struct VfsNode {
    VfsNode(std::string name, mode_t mode) : name(std::move(name)), mode(mode) {
        clock_gettime(CLOCK_REALTIME, &times[0]);
        clock_gettime(CLOCK_REALTIME, &times[1]);
    }

    timespec times[2]{};

    std::string name;
    mode_t mode{};

    virtual bool is_directory() {
        return false;
    }
};

struct File : public VfsNode {
    std::vector<uint8_t> content;

    File(std::string name, mode_t mode, std::vector<uint8_t> content = {})
        : VfsNode(std::move(name), mode), content(std::move(content)) {}

    File(std::string name, mode_t mode, std::string content = "")
        : VfsNode(std::move(name), mode), content(content.begin(), content.end()) {}

    // ...
};

struct Directory : public VfsNode {
    std::map<std::string, File> files;
    std::map<std::string, Directory> subdirectories;

    Directory(const std::string &name, mode_t mode) : VfsNode(name, mode) {}
    Directory &operator=(Directory const &other) = default;

    bool is_directory() override {
        return true;
    }

    void add_file(const std::string &name, mode_t mode, const std::vector<uint8_t> &content) {
        // files[name] = File(name, mode, content);
    }

    void add_file(const std::string &name, mode_t mode, const std::string &content) {
        // files[name] = File(name, mode, content);
    }

    void add_subdirectory(const std::string &name, mode_t mode) {
        // subdirectories[name] = Directory(name, mode);
    }
};

#endif  // SRC_VFS_NODE_H
