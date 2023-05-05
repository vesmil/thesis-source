#ifndef SRC_PATH_H
#define SRC_PATH_H
#include <algorithm>

class Path {
public:
    explicit Path(const std::string& path) : path_(normalize(path)) {}

    [[nodiscard]] Path parent() const {
        auto last_slash = path_.rfind('/');
        if (last_slash == std::string::npos || path_ == "/" || path_.empty()) {
            return Path("");
        } else if (last_slash == 0) {
            return Path("/");
        } else {
            return Path(path_.substr(0, last_slash));
        }
    }

    [[nodiscard]] Path basename() const {
        auto last_slash = path_.rfind('/');
        return last_slash == std::string::npos ? Path(path_) : Path(path_.substr(last_slash + 1));
    }

    [[nodiscard]] static std::string get_parent(const std::string& path) {
        auto last_slash = path.rfind('/');
        if (last_slash == std::string::npos || path == "/" || path.empty()) {
            return "";
        } else if (last_slash == 0) {
            return "/";
        } else {
            return path.substr(0, last_slash);
        }
    }

    [[nodiscard]] static std::string get_basename(const std::string& path) {
        auto last_slash = path.rfind('/');
        return last_slash == std::string::npos ? path : path.substr(last_slash + 1);
    }

    [[nodiscard]] std::string to_string() const {
        return path_;
    }

    Path operator+(const std::string& other) const {
        return Path(path_ + normalize(other));
    }

    Path operator+(const Path& other) const {
        return Path(path_ + normalize(other.path_));
    }

    Path& operator+=(const std::string& other) {
        path_ += normalize(other);
        return *this;
    }

    Path& operator+=(const Path& other) {
        path_ += normalize(other.path_);
        return *this;
    }

    bool operator==(const std::string& other) const {
        return path_ == normalize(other);
    }

    bool operator==(const Path& other) const {
        return path_ == other.path_;
    }

    bool operator!=(const std::string& other) const {
        return !(*this == other);
    }

    bool operator!=(const Path& other) const {
        return !(*this == other);
    }

private:
    static std::string normalize(const std::string& path) {
        if (path.empty()) return path;
        if (path == "/") return path;

        std::string result = path;
        if (result[0] != '/') {
            result.insert(result.begin(), '/');
        }
        if (result.back() == '/') {
            result.pop_back();
        }
        return result;
    }

    std::string path_;
};

#endif  // SRC_PATH_H
