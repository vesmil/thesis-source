#ifndef SRC_PREFIX_PARSER_H
#define SRC_PREFIX_PARSER_H

#include <string>
#include <vector>

#include "path.h"

class PrefixParser {
public:
    static std::string apply_prefix(const std::string& path, const std::string& module_name,
                                    const std::vector<std::string>& args) {
        Path parent = Path(path).parent();
        std::string basename = Path::string_basename(path);

        std::string arged_prefix = "#" + module_name;
        for (const auto& arg : args) {
            arged_prefix += "-" + arg;
        }

        return parent / (arged_prefix + "#" + basename);
    }

    static std::vector<std::string> args_from_prefix(const std::string& prefixed_path, const std::string& module_name) {
        std::string basename = Path::string_basename(prefixed_path);
        std::string prefix = "#" + module_name + "-";

        auto prefix_start = basename.find(prefix);
        if (prefix_start == std::string::npos) {
            return {};
        }

        auto prefix_end = basename.find('#', prefix_start + prefix.size());

        std::string args = basename.substr(prefix_start + prefix.size(), prefix_end - prefix_start - prefix.size());

        std::vector<std::string> args_vec;
        std::string arg;
        for (const auto& c : args) {
            if (c == '-') {
                args_vec.push_back(arg);
                arg = "";
            } else {
                arg += c;
            }
        }
        args_vec.push_back(arg);

        return args_vec;
    }

    static bool is_prefixed(const std::string& path) {
        std::string base = Path::string_basename(path);
        return std::count(base.begin(), base.end(), '#') % 2 == 0;
    }

    static std::string get_nonprefixed(const std::string& prefixed_path) {
        std::string result = prefixed_path;
        size_t prefixStart, prefixEnd;

        while (true) {
            prefixStart = result.find('#');
            if (prefixStart == std::string::npos) {
                break;
            }

            prefixEnd = result.find('#', prefixStart + 1);
            if (prefixEnd == std::string::npos) {
                break;
            }

            result.erase(prefixStart, prefixEnd - prefixStart + 1);
        }

        return result;
    }
};

#endif  // SRC_PREFIX_PARSER_H
