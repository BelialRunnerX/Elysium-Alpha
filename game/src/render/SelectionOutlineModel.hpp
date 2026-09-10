#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build presentation records for hover, selection, targeting, squad, build, interact, and danger highlights.
struct SelectionOutlineModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SelectionOutlineModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SelectionOutlineModelStore { public: bool apply(const SelectionOutlineModelOp&); bool erase(std::uint64_t); const SelectionOutlineModelData* find(std::uint64_t) const; std::vector<SelectionOutlineModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SelectionOutlineModelData> data_; };
}
