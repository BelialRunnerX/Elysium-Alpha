// Intended function: common numeric-safety primitives for values amplified by layered simulation systems.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace elysium::safe {

// Published continuous simulation values use a conservative integer-shaped ceiling.
// This keeps values finite and leaves a straightforward path to fixed-point / uint32
// persistence without allowing accidental infinities to escape subsystem boundaries.
inline constexpr double PublishedScalarCeiling =
    static_cast<double>(std::numeric_limits<std::uint32_t>::max());
inline constexpr float PublishedScalarCeilingF =
    static_cast<float>(std::numeric_limits<std::uint32_t>::max());

template <class T>
[[nodiscard]] constexpr T clamp(T value, T low, T high) noexcept {
    return value < low ? low : (value > high ? high : value);
}

[[nodiscard]] inline double finiteClamp(
    double value,
    double low,
    double high) noexcept {
    if (std::isnan(value)) return clamp(0.0, low, high);
    if (value == std::numeric_limits<double>::infinity()) return high;
    if (value == -std::numeric_limits<double>::infinity()) return low;
    return std::clamp(value, low, high);
}

[[nodiscard]] inline float finiteClamp(
    float value,
    float low,
    float high) noexcept {
    if (std::isnan(value)) return clamp(0.0f, low, high);
    if (value == std::numeric_limits<float>::infinity()) return high;
    if (value == -std::numeric_limits<float>::infinity()) return low;
    return std::clamp(value, low, high);
}

[[nodiscard]] inline double nonNegative(
    double value,
    double ceiling = PublishedScalarCeiling) noexcept {
    const double safeCeiling = (!std::isfinite(ceiling) || ceiling < 0.0)
        ? PublishedScalarCeiling
        : std::min(ceiling, PublishedScalarCeiling);
    return finiteClamp(value, 0.0, safeCeiling);
}

[[nodiscard]] inline float nonNegative(
    float value,
    float ceiling = PublishedScalarCeilingF) noexcept {
    const float safeCeiling = (!std::isfinite(ceiling) || ceiling < 0.0f)
        ? PublishedScalarCeilingF
        : std::min(ceiling, PublishedScalarCeilingF);
    return finiteClamp(value, 0.0f, safeCeiling);
}

template <class T>
requires(std::is_unsigned_v<T>)
[[nodiscard]] constexpr T saturatingAdd(T a, T b) noexcept {
    const T max = std::numeric_limits<T>::max();
    return b > max - a ? max : static_cast<T>(a + b);
}

template <class T>
requires(std::is_unsigned_v<T>)
[[nodiscard]] constexpr T saturatingMultiply(T a, T b) noexcept {
    if (a == 0 || b == 0) return 0;
    const T max = std::numeric_limits<T>::max();
    return a > max / b ? max : static_cast<T>(a * b);
}

template <class To, class From>
requires(std::is_integral_v<To> && std::is_integral_v<From>)
[[nodiscard]] constexpr To saturatingCast(From value) noexcept {
    using ToLimits = std::numeric_limits<To>;
    if constexpr (std::is_signed_v<From> == std::is_signed_v<To>) {
        if (value < static_cast<From>(ToLimits::lowest())) return ToLimits::lowest();
        if (value > static_cast<From>(ToLimits::max())) return ToLimits::max();
        return static_cast<To>(value);
    } else if constexpr (std::is_signed_v<From>) {
        if (value <= 0) return To{0};
        using UFrom = std::make_unsigned_t<From>;
        const UFrom u = static_cast<UFrom>(value);
        if (u > static_cast<UFrom>(ToLimits::max())) return ToLimits::max();
        return static_cast<To>(u);
    } else {
        using UTo = std::make_unsigned_t<To>;
        if (value > static_cast<From>(static_cast<UTo>(ToLimits::max()))) return ToLimits::max();
        return static_cast<To>(value);
    }
}

[[nodiscard]] inline std::uint64_t saturatingIncrement(std::uint64_t value) noexcept {
    return saturatingAdd<std::uint64_t>(value, 1ULL);
}

} // namespace elysium::safe
