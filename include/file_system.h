#include <memory>
#include <string>
#include <vector>
#include "node.h"

class FileSystem {
public:
    FileSystem(const std::string& mountPoint);
    ~FileSystem();

    std::shared_ptr<Node> getRoot() const;
    std::shared_ptr<Node> getNode(const std::string& path) const;
    std::vector<std::shared_ptr<Node>> getChildren(const std::string& path) const;

    bool createDirectory(const std::string& path);
    bool createFile(const std::string& path);

    // Should this be on node level? ...
    bool remove(const std::string& path);
    bool rename(const std::string& oldPath, const std::string& newPath);
    bool write(const std::string& path, const std::string& contents);

private:
    std::string mountPoint_;
    // TODO: Add data members for storing nodes and file system metadata
};