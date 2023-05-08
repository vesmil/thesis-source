#ifndef SRC_VFS_DECORATOR_H
#define SRC_VFS_DECORATOR_H

#include <memory>

#include "custom_vfs.h"

/**
 * @summary A decorator for a CustomVfs that allows to add functionality to the wrapped VFS while keeping the same
 * interface.
 *
 * @see https://refactoring.guru/design-patterns/decorator/cpp/example
 */
class VfsDecorator : public CustomVfs {
public:
    explicit VfsDecorator(CustomVfs &wrapped_vfs) : CustomVfs(wrapped_vfs), wrapped_vfs(wrapped_vfs) {}

protected:
    [[nodiscard]] CustomVfs &get_wrapped() {
        return wrapped_vfs;
    }

    [[nodiscard]] const CustomVfs &get_wrapped() const {
        return wrapped_vfs;
    }

private:
    CustomVfs &wrapped_vfs;
};

#endif  // SRC_VFS_DECORATOR_H
