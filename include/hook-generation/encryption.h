#ifndef SRC_ENCRYPTION_H
#define SRC_ENCRYPTION_H

#include <string>

#include "common/config.h"
#include "common/prefix_parser.h"

/// Encryption tool for generating hooks
namespace EncryptionHookGenerator {

inline std::string lock_pass_hook(const std::string& filename) {
    return PrefixParser::apply_prefix(filename, Config::encryption.prefix, {"lock"});
}

inline std::string unlock_pass_hook(const std::string& filename) {
    return PrefixParser::apply_prefix(filename, Config::encryption.prefix, {"unlock"});
}

inline std::string lock_file_hook(const std::string& filename, const std::string& key_path) {
    return PrefixParser::apply_prefix(filename, Config::encryption.prefix, {"lock", key_path});
}

inline std::string unlock_file_hook(const std::string& filename, const std::string& key_path) {
    return PrefixParser::apply_prefix(filename, Config::encryption.prefix, {"unlock", key_path});
}

inline std::string generate_key_hook(const std::string& filename) {
    return PrefixParser::apply_prefix(filename, Config::encryption.prefix, {"generate"});
}

inline std::string set_key_path_hook(const std::string& filename, const std::string& key_path) {
    return PrefixParser::apply_prefix(filename, Config::encryption.prefix, {"set", key_path});
}

}  // namespace EncryptionHookGenerator

#endif  // SRC_ENCRYPTION_H
