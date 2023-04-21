#include <filesystem>

#include "custom_vfs.h"
#include "encryption_decorator.h"
#include "versioning_vfs.h"

int main(int argc, char* argv[]) {
    std::string mountpoint = argv[argc - 1];
    if (!std::filesystem::is_directory(mountpoint)) {
        std::cerr << "Mountpoint is not a directory" << std::endl;
        return 1;
    }

    CustomVfs custom_vfs;

    if (std::filesystem::is_empty(mountpoint)) {
        custom_vfs = CustomVfs(true);
    } else {
        custom_vfs = CustomVfs(mountpoint);
    }

    VersioningVfs versioned = VersioningVfs(custom_vfs);
    EncryptionVfs encrypted = EncryptionVfs(versioned);

    encrypted.main(argc, argv);

    return 0;
}
