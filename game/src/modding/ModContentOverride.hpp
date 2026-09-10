#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent explicit content replacement/extension rules with provenance and deterministic conflict reporting.
struct ModContentOverrideOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ModContentOverrideData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ModContentOverrideStore { public: bool apply(const ModContentOverrideOp&); bool erase(std::uint64_t); const ModContentOverrideData* find(std::uint64_t) const; std::vector<ModContentOverrideData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ModContentOverrideData> data_; };
}
