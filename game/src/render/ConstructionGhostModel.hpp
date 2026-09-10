#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build renderer-neutral blueprint preview, validity, resource, support, utility, and obstruction visualization records.
struct ConstructionGhostModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ConstructionGhostModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ConstructionGhostModelStore { public: bool apply(const ConstructionGhostModelOp&); bool erase(std::uint64_t); const ConstructionGhostModelData* find(std::uint64_t) const; std::vector<ConstructionGhostModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ConstructionGhostModelData> data_; };
}
