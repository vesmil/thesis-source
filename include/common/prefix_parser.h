#ifndef SRC_PREFIX_PARSER_H
#define SRC_PREFIX_PARSER_H

#include <string>
#include <vector>

#include "path.h"

class PrefixParser {
public:
    static std::string apply_prefix(const std::string& path, const std::string& module_name,
                                    const std::vector<std::string>& args = {});
    static std::vector<std::string> args_from_prefix(const std::string& prefixed_path, const std::string& module_name);

    static bool is_prefixed(const std::string& path);
    static std::string get_nonprefixed(const std::string& prefixed_path);
};

#endif  // SRC_PREFIX_PARSER_H
