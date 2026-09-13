#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track architectural styles, materials, spatial preferences, symbolic forms, climate adaptation, and faction identity.
struct ArchitectureTraditionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ArchitectureTraditionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ArchitectureTraditionSystemStore { public: bool apply(const ArchitectureTraditionSystemOp&); bool erase(std::uint64_t); const ArchitectureTraditionSystemData* find(std::uint64_t) const; std::vector<ArchitectureTraditionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ArchitectureTraditionSystemData> data_; };
}
