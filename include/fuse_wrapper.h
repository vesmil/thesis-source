#include <fuse.h>
#include <vector>
#include <string>
#include <memory>
#include <iostream>

template<class Root>
struct FuseWrapper {
    Root root_;

    static FuseWrapper& instance() {
        static FuseWrapper instance;
        return instance;
    }

    int run(int argc, char* argv[], bool default_options = true) {
        std::vector<char*> args(argv, argv + argc);

        if (default_options) {
            args.push_back(const_cast<char*>("-s"));
            args.push_back(const_cast<char*>("-o"));
            args.push_back(const_cast<char*>("default_permissions"));
        }

        return fuse_main(static_cast<int>(args.size()), args.data(), FuseWrapper::getOperations());
    }

    static fuse_operations_compat2 ops;
    static const fuse_operations_compat2* getOperations() {
        ops = {};
        ops.getattr = &FuseWrapper::getattr;
        ops.mkdir = &FuseWrapper::mkdir;
        ops.read = &FuseWrapper::read;
        ops.write = &FuseWrapper::write;


        return &ops;
    }

    static int getattr(const char* path, struct stat* stbuf) {
        return FuseWrapper::instance().root_.getattr(path, stbuf);
    }

    static int mkdir(const char* path, mode_t mode) {
        return FuseWrapper::instance().root_.mkdir(path, mode);
    }

    static int read(const char* path, char* buf, size_t size, off_t offset) {
        return FuseWrapper::instance().root_.read(path, buf, size, offset);
    }

    static int write(const char* path, const char* buf, size_t size,
                     off_t offset) {
        return FuseWrapper::instance().root_.write(path, buf, size, offset);
    }
};
