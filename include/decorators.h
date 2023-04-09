#ifndef SRC_DECORATORS_H
#define SRC_DECORATORS_H

#include <cstdio>
#include <memory>
#include <utility>

#include "nodes.h"

class VfsNodeDecorator : public VfsNode {
public:
    explicit VfsNodeDecorator(std::shared_ptr<VfsNode> vfs_node)
        : vfs_node_(std::move(vfs_node)), VfsNode(vfs_node->name, vfs_node->mode) {}
    virtual ~VfsNodeDecorator() = default;

protected:
    std::shared_ptr<VfsNode> vfs_node_;
};

class EncryptionDecorator : public VfsNodeDecorator {
public:
    explicit EncryptionDecorator(std::shared_ptr<VfsNode> vfs_node) : VfsNodeDecorator(std::move(vfs_node)) {}
    // TODO Overridden methods for encryption and decryption
};

class VersioningDecorator : public VfsNodeDecorator {
public:
    explicit VersioningDecorator(std::shared_ptr<VfsNode> vfs_node) : VfsNodeDecorator(std::move(vfs_node)) {}
    // TODO Overridden methods for versioning
};

#endif  // SRC_DECORATORS_H
