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

inline std::string default_lock_hook(const std::string& filename) {
    return PrefixParser::apply_prefix(filename, Config::encryption.prefix, {"defaultLock"});
}

inline std::string lock_key_hook(const std::string& filename, const std::string& key_path) {
    std::string key_path_escaped = key_path;
    std::replace(key_path_escaped.begin(), key_path_escaped.end(), '/', '|');
    return PrefixParser::apply_prefix(filename, Config::encryption.prefix, {"lock", key_path_escaped});
}

inline std::string unlock_key_hook(const std::string& filename, const std::string& key_path) {
    std::string key_path_escaped = key_path;
    std::replace(key_path_escaped.begin(), key_path_escaped.end(), '/', '|');
    return PrefixParser::apply_prefix(filename, Config::encryption.prefix, {"unlock", key_path_escaped});
}

inline std::string generate_key_hook(const std::string& filename) {
    return PrefixParser::apply_prefix(filename, Config::encryption.prefix, {"generate"});
}

inline std::string set_key_path_hook(const std::string& vfs, const std::string& key_path) {
    std::string key_path_escaped = key_path;
    std::replace(key_path_escaped.begin(), key_path_escaped.end(), '/', '|');
    return PrefixParser::apply_prefix(vfs, Config::encryption.prefix, {"setDefault", key_path_escaped});
}

}  // namespace EncryptionHookGenerator

#endif  // SRC_ENCRYPTION_H
