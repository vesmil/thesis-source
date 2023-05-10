#ifndef SRC_VERSIONING_H
#define SRC_VERSIONING_H

#include <string>

#include "common/config.h"
#include "common/prefix_parser.h"

/// Versioning tool for generating hooks
namespace VersioningHookGenerator {

inline std::string list_hook(const std::string& filename) {
    return PrefixParser::apply_prefix(filename, Config::versioning.prefix, {"list"});
}

inline std::string restore_hook(const std::string& filename, const std::string& version) {
    return PrefixParser::apply_prefix(filename, Config::versioning.prefix, {"restore", version});
}

inline std::string delete_hook(const std::string& filename, const std::string& version) {
    return PrefixParser::apply_prefix(filename, Config::versioning.prefix, {"delete", version});
}

inline std::string delete_all_hook(const std::string& filename) {
    return PrefixParser::apply_prefix(filename, Config::versioning.prefix, {"deleteAll"});
}

}  // namespace VersioningHookGenerator

#endif  // SRC_VERSIONING_H
