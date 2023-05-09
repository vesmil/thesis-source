#ifndef SRC_PREFIX_PARSER_H
#define SRC_PREFIX_PARSER_H

#include <string>
#include <vector>

#include "path.h"

class PrefixParser {
public:
    /**
     * @brief Applies a prefix to a path
     * @example apply_prefix("/home/user/file.txt", "ENCRYPTION", {"1"}) -> "#ENCRYPTION-1#/home/user/file.txt"
     */
    static std::string apply_prefix(const std::string& path, const std::string& module_name,
                                    const std::vector<std::string>& args = {});

    /**
     * @brief Removes a specific prefix from a path
     * @example remove_specific_prefix("#ENCRYPTION-1##VERSIONING-#/file.txt", "ENCRYPTION") -> "#VERSIONING-#/file.txt"
     */
    static std::string remove_specific_prefix(std::string path, const std::string& prefix);

    /**
     * @brief Returns the arguments from a prefixed path
     * @example args_from_prefix("#ENCRYPTION-1#/home/user/file.txt", "ENCRYPTION") -> {"1"}
     */
    static std::vector<std::string> args_from_prefix(const std::string& prefixed_path, const std::string& module_name);

    /**
     * @brief Returns whether a path contains a prefix
     * @example contains_prefix("#ENCRYPTION-1#/home/user/file.txt", "ENCRYPTION") -> true
     */
    static bool contains_prefix(const std::string& path, const std::string& module_name);

    /**
     * @brief Returns whether a path is prefixed with anything
     */
    static bool is_prefixed(const std::string& path);

    /**
     * @brief Returns the path without the prefix
     * @example get_nonprefixed("#ENCRYPTION-1#/home/user/file.txt") -> "/home/user/file.txt"
     */
    static std::string get_nonprefixed(const std::string& prefixed_path);
};

#endif  // SRC_PREFIX_PARSER_H
