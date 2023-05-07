#ifndef SRC_PATH_H
#define SRC_PATH_H

#include <algorithm>
#include <filesystem>

class Path {
public:
    explicit Path(const std::string& path) : path_(normalize(path)) {}
    Path() = default;

    [[nodiscard]] Path parent() const;
    [[nodiscard]] Path basename() const;

    [[nodiscard]] static Path to_absolute(const std::string& path);

    [[nodiscard]] static std::string string_parent(const std::string& path);
    [[nodiscard]] static std::string string_basename(const std::string& path);

    [[nodiscard]] std::string to_string() const;
    [[nodiscard]] const char* c_str() const;

    operator std::string() const;  // NOLINT(google-explicit-constructor)

    Path operator/(const std::string& other) const;
    Path operator/(const Path& other) const;
    Path& operator/=(const std::string& other);
    Path& operator/=(const Path& other);
    bool operator==(const std::string& other) const;
    bool operator==(const Path& other) const;
    bool operator!=(const std::string& other) const;

    bool operator!=(const Path& other) const;

private:
    static std::string normalize(const std::string& path);

    std::string path_;
};

#endif  // SRC_PATH_H
