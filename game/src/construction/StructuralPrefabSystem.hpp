#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent reusable structural prefab assemblies with stable part IDs, supports, utilities, and damage hooks.
struct StructuralPrefabSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct StructuralPrefabSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class StructuralPrefabSystemStore { public: bool apply(const StructuralPrefabSystemOp&); bool erase(std::uint64_t); const StructuralPrefabSystemData* find(std::uint64_t) const; std::vector<StructuralPrefabSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StructuralPrefabSystemData> data_; };
}
