#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track prosthetic limbs/organs, fit, quality, damage, repair, mobility, work, and combat modifiers.
struct ProstheticSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ProstheticSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ProstheticSystemStore { public: bool apply(const ProstheticSystemOp&); bool erase(std::uint64_t); const ProstheticSystemData* find(std::uint64_t) const; std::vector<ProstheticSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ProstheticSystemData> data_; };
}
