#ifndef SRC_VFS_DECORATOR_H
#define SRC_VFS_DECORATOR_H

#include <memory>

#include "custom_vfs.h"

class VfsDecorator : public CustomVfs {
public:
    explicit VfsDecorator(CustomVfs& wrapped_vfs) : wrapped_vfs_(wrapped_vfs) {}

protected:
    CustomVfs& wrapped_vfs_;
};

#endif  // SRC_VFS_DECORATOR_H
