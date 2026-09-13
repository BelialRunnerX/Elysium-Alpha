#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent very-large construction projects as bounded phases with resource, labor, logistics, and strategic dependencies.
struct MegastructureProjectSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct MegastructureProjectSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class MegastructureProjectSystemStore { public: bool apply(const MegastructureProjectSystemOp&); bool erase(std::uint64_t); const MegastructureProjectSystemData* find(std::uint64_t) const; std::vector<MegastructureProjectSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MegastructureProjectSystemData> data_; };
}
