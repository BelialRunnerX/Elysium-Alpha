#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build renderer-neutral localized gouge, scorch, crack, breach, blood, dirt, and repair-mark presentation records.
struct DamageDecalModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct DamageDecalModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class DamageDecalModelStore { public: bool apply(const DamageDecalModelOp&); bool erase(std::uint64_t); const DamageDecalModelData* find(std::uint64_t) const; std::vector<DamageDecalModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DamageDecalModelData> data_; };
}
