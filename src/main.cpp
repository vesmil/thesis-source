#include <filesystem>

#include "custom_vfs.h"

int main(int argc, char* argv[]) {
    std::string mountpoint = argv[argc - 1];
    if (!std::filesystem::is_directory(mountpoint)) {
        std::cerr << "Mountpoint is not a directory" << std::endl;
        return 1;
    }

    CustomVfs fuseWrapper(mountpoint);

    fuseWrapper.main(argc, argv);
    fuseWrapper.test_files();

    return 0;
}
