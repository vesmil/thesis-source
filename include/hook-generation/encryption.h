#ifndef SRC_ENCRYPTION_H
#define SRC_ENCRYPTION_H

#include <string>

#include "common/prefix_parser.h"

/// Encryption tool for generating hooks
namespace Encryption {

std::string prefix = "ENCRYPTION";

std::string lock_pass_hook(const std::string& filename) {
    return PrefixParser::apply_prefix(filename, prefix, {"lock"});
}
std::string unlock_pass_hook(const std::string& filename) {
    return PrefixParser::apply_prefix(filename, prefix, {"unlock"});
}
std::string lock_file_hook(const std::string& filename, const std::string& key_path) {
    return PrefixParser::apply_prefix(filename, prefix, {"lock", key_path});
}
std::string unlock_file_hook(const std::string& filename, const std::string& key_path) {
    return PrefixParser::apply_prefix(filename, prefix, {"unlock", key_path});
}
std::string generate_key_hook(const std::string& filename) {
    return PrefixParser::apply_prefix(filename, prefix, {"generate"});
}
std::string set_key_path_hook(const std::string& filename, const std::string& key_path) {
    return PrefixParser::apply_prefix(filename, prefix, {"set", key_path});
}

}  // namespace Encryption

#endif  // SRC_ENCRYPTION_H
