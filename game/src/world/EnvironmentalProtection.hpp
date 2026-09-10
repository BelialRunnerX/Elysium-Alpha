#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent protected zones, conservation rules, extraction limits, species protection, and faction consequences.
struct EnvironmentalProtectionOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct EnvironmentalProtectionData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class EnvironmentalProtectionStore { public: bool apply(const EnvironmentalProtectionOp&); bool erase(std::uint64_t); const EnvironmentalProtectionData* find(std::uint64_t) const; std::vector<EnvironmentalProtectionData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,EnvironmentalProtectionData> data_; };
}
