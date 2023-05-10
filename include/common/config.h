#ifndef SRC_CONFIG_H
#define SRC_CONFIG_H

#include <cstddef>
#include <string>

#include "path.h"

/**
 * @brief Namespace with global configurations
 *
 * @note Should be replaced with a proper configuration library in the future
 */
namespace Config {

/// @brief Configuration class for the base filesystem
struct Base {
    std::string backing_location = "/mnt/";
    std::string backing_prefix = "customvfs-";
};

/// @brief Configuration class for the versioning filesystem
struct Versioning {
    std::string prefix = "VERSION";
};

/// @brief Configuration class for the encryption filesystem
struct Encryption {
    std::string prefix = "ENCRYPTION";
    std::string path_to_key_path = "/#ENCRYPTION-keyPath#path";
};

static Base base;
static Versioning versioning;
static Encryption encryption;

}  // namespace Config

#endif  // SRC_CONFIG_H
