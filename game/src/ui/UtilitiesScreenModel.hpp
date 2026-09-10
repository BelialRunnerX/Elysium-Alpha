#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build power, atmosphere, fluid, heat, data, waste, alarms, load, reserve, and priority projections.
struct UtilitiesScreenModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct UtilitiesScreenModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class UtilitiesScreenModelStore { public: bool apply(const UtilitiesScreenModelOp&); bool erase(std::uint64_t); const UtilitiesScreenModelData* find(std::uint64_t) const; std::vector<UtilitiesScreenModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,UtilitiesScreenModelData> data_; };
}
