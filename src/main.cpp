#include <iostream>

#include "file_system.h"
#include "fuse_wrapper.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    return FuseWrapper::instance().run();

    return 0;
}
