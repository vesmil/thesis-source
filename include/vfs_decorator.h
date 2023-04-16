#ifndef SRC_VFS_DECORATOR_H
#define SRC_VFS_DECORATOR_H

#include <memory>

#include "custom_vfs.h"

class VfsDecorator : public CustomVfs {
public:
    explicit VfsDecorator(std::shared_ptr<CustomVfs> wrapped_vfs) : wrapped_vfs_(std::move(wrapped_vfs)) {}

protected:
    std::shared_ptr<CustomVfs> wrapped_vfs_;
};

#endif  // SRC_VFS_DECORATOR_H
