#ifndef SRC_PATH_H
#define SRC_PATH_H

#include <algorithm>
#include <filesystem>

/**
 * @brief Custom lightweight class to handle paths with implicit conversion to std::string
 */
class Path {
public:
    explicit Path(const std::string& path) : path_(normalize(path)) {}
    Path() = default;

    /// @brief Returns the Path to parent directory
    [[nodiscard]] Path parent() const;

    /// @brief Returns the basename as a Path
    [[nodiscard]] Path basename() const;

    /// @brief Returns the absolute Path
    [[nodiscard]] static Path to_absolute(const std::string& path);

    /// @brief Returns path to parent directory as a string
    [[nodiscard]] static std::string string_parent(const std::string& path);

    /// @brief Returns basename as a string
    [[nodiscard]] static std::string string_basename(const std::string& path);

    /// @brief Explicitly converts to std::string
    [[nodiscard]] std::string to_string() const;

    /// @brief Implicitly converts to std::string
    [[nodiscard]] operator std::string() const;  // NOLINT(google-explicit-constructor)

    /// @brief Returns the path as a C string
    [[nodiscard]] const char* c_str() const;

    /// @brief Concatenates two paths
    Path operator/(const std::string& other) const;

    /// @brief Concatenates two paths
    Path operator/(const Path& other) const;

    /// @brief Concatenates two paths
    Path& operator/=(const std::string& other);

    /// @brief Concatenates two paths
    Path& operator/=(const Path& other);

    bool operator==(const std::string& other) const;
    bool operator==(const Path& other) const;
    bool operator!=(const std::string& other) const;
    bool operator!=(const Path& other) const;

private:
    /// @brief Returns path in normalized form (no trailing slashes, etc.)
    [[nodiscard]] static std::string normalize(const std::string& path);

    std::string path_;
};

#endif  // SRC_PATH_H
