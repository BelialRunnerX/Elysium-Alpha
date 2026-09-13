#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track deterministic stellar age, luminosity, activity, evolution stage, hazards, and long-horizon changes.
struct StellarLifecycleOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct StellarLifecycleData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class StellarLifecycleStore { public: bool apply(const StellarLifecycleOp&); bool erase(std::uint64_t); const StellarLifecycleData* find(std::uint64_t) const; std::vector<StellarLifecycleData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StellarLifecycleData> data_; };
}
