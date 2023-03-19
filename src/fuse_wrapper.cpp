#include "fuse_wrapper.h"

FuseWrapper& FuseWrapper::instance() {
    static FuseWrapper instance;
    return instance;
}

int FuseWrapper::run(int argc, char *argv[], bool default_options) {
    std::vector<char*> args(argv, argv + argc);

    if (default_options) {
        args.push_back(const_cast<char*>("-s"));
        args.push_back(const_cast<char*>("-o"));
        args.push_back(const_cast<char*>("default_permissions"));
    }

    return fuse_main(static_cast<int>(args.size()), args.data(), FuseWrapper::getOperations());
}

int FuseWrapper::getattr(const char *path, struct stat *stbuf) {
    // Implement your getattr function here
    return 0;
}

int FuseWrapper::mkdir(const char *path, mode_t mode) {
    // Implement your mkdir function here
    return 0;
}

int FuseWrapper::read(const char *path, char *buf, size_t size, off_t offset) {
    return 0;
}
int FuseWrapper::write(const char *path, const char *buf, size_t size,
                       off_t offset) {
    return 0;
}

const fuse_operations* FuseWrapper::getOperations() {
    ops = {};
    ops.getattr = &FuseWrapper::getattr;
    ops.mkdir = &FuseWrapper::mkdir;
    ops.read = &FuseWrapper::read;
    ops.write = &FuseWrapper::write;

    return &ops;
}

fuse_operations FuseWrapper::ops;
