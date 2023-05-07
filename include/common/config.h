#ifndef SRC_CONFIG_H
#define SRC_CONFIG_H

#include <cstddef>
#include <string>

#include "path.h"

/**
 * Configuration for filesystems
 */
namespace Config {

/**
 * Class to load configuration from a file
 */
class Parser {
public:
    static bool ParseFile(const std::string &path);
};

struct Base {
    std::string backing_location = "/mnt/";
    std::string backing_prefix = "customvfs-";
};

struct Versioning {};
struct Encryption {};

static Base base;

// static Versioning versioning;

// static Encryption encryption;

}  // namespace Config

#endif  // SRC_CONFIG_H
