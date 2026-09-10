#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build citizen needs, skills, profession, relationships, health, memories, schedule, equipment, and orders projections.
struct CitizenScreenModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CitizenScreenModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CitizenScreenModelStore { public: bool apply(const CitizenScreenModelOp&); bool erase(std::uint64_t); const CitizenScreenModelData* find(std::uint64_t) const; std::vector<CitizenScreenModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CitizenScreenModelData> data_; };
}
