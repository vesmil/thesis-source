#include "common/prefix_parser.h"

std::string PrefixParser::apply_prefix(const std::string& path, const std::string& module_name,
                                       const std::vector<std::string>& args) {
    Path parent = Path(path).parent();
    std::string basename = Path::string_basename(path);

    for (const auto& arg : args) {
        if (arg.find('-') != std::string::npos || arg.find('#') != std::string::npos) {
            throw std::runtime_error("PrefixParser::apply_prefix: args cannot contain '-' or '#'");
        }
    }

    std::string module_prefix = "#" + module_name;
    for (const auto& arg : args) {
        module_prefix += "-" + arg;
    }

    return parent / (module_prefix + "#" + basename);
}

std::string PrefixParser::remove_specific_prefix(std::string path, const std::string& prefix) {
    auto prefix_start = path.find(prefix);
    if (prefix_start == std::string::npos) {
        return path;
    }

    auto prefix_end = path.find('#', prefix_start + prefix.size());
    return path.substr(0, prefix_start - 1) + path.substr(prefix_end + 1);
}

std::vector<std::string> PrefixParser::args_from_prefix(const std::string& prefixed_path,
                                                        const std::string& module_name) {
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

bool PrefixParser::is_prefixed(const std::string& path) {
    std::string base = Path::string_basename(path);

    std::size_t hash_count = std::count(base.begin(), base.end(), '#');
    std::size_t hyphen_count = std::count(base.begin(), base.end(), '-');

    return hash_count % 2 == 0 && hash_count > 0 && hyphen_count >= hash_count / 2;
}

std::string PrefixParser::get_nonprefixed(const std::string& prefixed_path) {
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

bool PrefixParser::contains_prefix(const std::string& path, const std::string& module_name) {
    std::string base = Path::string_basename(path);

    if (!is_prefixed(path)) {
        return false;
    }

    std::string prefix_1 = "#" + module_name + "#";
    std::string prefix_2 = "#" + module_name + "-";

    return base.find(prefix_1) != std::string::npos || base.find(prefix_2) != std::string::npos;
}
