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

    std::shared_ptr<CustomVfs> custom_vfs;

    if (std::filesystem::is_empty(mountpoint)) {
            custom_vfs = std::make_shared<CustomVfs>(true);
    } else {
            custom_vfs = std::make_shared<CustomVfs>(mountpoint);
    }

    std::shared_ptr<VersioningVfs> versioned = std::make_shared<VersioningVfs>(custom_vfs);
    std::shared_ptr<EncryptionVfs> encrypted = std::make_shared<EncryptionVfs>(versioned);

    encrypted->main(argc, argv);

    return 0;
}
