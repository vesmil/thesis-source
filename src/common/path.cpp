#include "common/path.h"

Path Path::parent() const {
    auto last_slash = path_.rfind('/');
    if (last_slash == std::string::npos || path_ == "/" || path_.empty()) {
        return Path("");
    } else if (last_slash == 0) {
        return Path("/");
    } else {
        return Path(path_.substr(0, last_slash));
    }
}

Path Path::to_absolute(const std::string& path) {
    return Path(std::filesystem::absolute(path).generic_string());
}

Path Path::basename() const {
    auto last_slash = path_.rfind('/');
    return last_slash == std::string::npos ? Path(path_) : Path(path_.substr(last_slash + 1));
}

std::string Path::string_parent(const std::string& path) {
    auto last_slash = path.rfind('/');
    if (last_slash == std::string::npos || path == "/" || path.empty()) {
        return "";
    } else if (last_slash == 0) {
        return "/";
    } else {
        return path.substr(0, last_slash);
    }
}

std::string Path::string_basename(const std::string& path) {
    auto last_slash = path.rfind('/');
    return last_slash == std::string::npos ? path : path.substr(last_slash + 1);
}

const char* Path::c_str() const {
    return path_.c_str();
}

Path::operator std::string() const {
    return path_;
}

std::string Path::to_string() const {
    return path_;
}

Path Path::operator/(const Path& other) const {
    if (other.path_.empty()) return Path(path_);
    if (path_.empty()) return Path(other.path_);
    if (path_ == "/") return Path(other.path_);

    return Path(path_ + normalize(other.path_));
}

Path Path::operator/(const std::string& other) const {
    return Path(path_) / Path(other);
}

Path& Path::operator/=(const std::string& other) {
    path_ += normalize(other);
    return *this;
}
Path& Path::operator/=(const Path& other) {
    path_ += normalize(other.path_);
    return *this;
}
bool Path::operator==(const std::string& other) const {
    return path_ == normalize(other);
}
bool Path::operator==(const Path& other) const {
    return path_ == other.path_;
}
bool Path::operator!=(const std::string& other) const {
    return !(*this == other);
}
bool Path::operator!=(const Path& other) const {
    return !(*this == other);
}

std::string Path::normalize(const std::string& path) {
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
