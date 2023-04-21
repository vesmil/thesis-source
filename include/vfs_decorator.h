#ifndef SRC_VFS_DECORATOR_H
#define SRC_VFS_DECORATOR_H

#include <memory>

#include "custom_vfs.h"

/**
 * @summary A decorator for a CustomVfs that allows to add functionality to the wrapped VFS while keeping the same
 * interface. All non-overridden methods are forwarded to the wrapped VFS.
 * @see https://refactoring.guru/design-patterns/decorator/cpp/example
 */
class VfsDecorator : public CustomVfs {
public:
    explicit VfsDecorator(CustomVfs& wrapped_vfs) : wrapped_vfs(wrapped_vfs), CustomVfs(wrapped_vfs) {}

protected:
    CustomVfs& wrapped_vfs;
};

#endif  // SRC_VFS_DECORATOR_H
