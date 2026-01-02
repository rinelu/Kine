#pragma once

#include <cstdint>
#include <string>

namespace kine
{

// Semantic versioning: MAJOR.MINOR.PATCH

struct Version
{
    std::uint32_t major = 0;
    std::uint32_t minor = 0;
    std::uint32_t patch = 0;

    constexpr Version() = default;
    constexpr Version(std::uint32_t maj, std::uint32_t min, std::uint32_t pat) : major(maj), minor(min), patch(pat) {}

    constexpr bool operator==(const Version& other) const
    {
        return major == other.major && minor == other.minor && patch == other.patch;
    }

    constexpr bool operator<(const Version& other) const
    {
        if (major != other.major) return major < other.major;
        if (minor != other.minor) return minor < other.minor;
        return patch < other.patch;
    }

    constexpr bool operator!=(const Version& other) const { return !(*this == other); }
    constexpr bool operator<=(const Version& other) const { return *this < other || *this == other; }
    constexpr bool operator>(const Version& other) const { return other < *this; }
    constexpr bool operator>=(const Version& other) const { return !(*this < other); }

    std::string to_string() const
    {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }
};

inline constexpr Version ENGINE_VERSION{1, 1, 0};

inline constexpr std::uint32_t ENGINE_VERSION_MAJOR = ENGINE_VERSION.major;
inline constexpr std::uint32_t ENGINE_VERSION_MINOR = ENGINE_VERSION.minor;
inline constexpr std::uint32_t ENGINE_VERSION_PATCH = ENGINE_VERSION.patch;

inline std::string engine_version_string() { return ENGINE_VERSION.to_string(); }

}  // namespace kine
