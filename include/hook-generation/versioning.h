#ifndef SRC_VERSIONING_H
#define SRC_VERSIONING_H

#include <string>

#include "common/prefix_parser.h"

/// Versioning tool for generating hooks
namespace Versioning {

std::string prefix = "VERSION";

std::string list_hook(const std::string& filename) {
    return PrefixParser::apply_prefix(filename, prefix, {"list"});
}

std::string restore_hook(const std::string& filename, const std::string& version) {
    return PrefixParser::apply_prefix(filename, prefix, {"restore", version});
}

std::string delete_hook(const std::string& filename, const std::string& version) {
    return PrefixParser::apply_prefix(filename, prefix, {"delete", version});
}

std::string delete_all_hook(const std::string& filename) {
    return PrefixParser::apply_prefix(filename, prefix, {"deleteAll"});
}

};  // namespace Versioning

#endif  // SRC_VERSIONING_H
