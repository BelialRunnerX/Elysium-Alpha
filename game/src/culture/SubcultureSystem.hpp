#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track emergent occupational, youth, military, scientific, criminal, frontier, and ideological subcultures.
struct SubcultureSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SubcultureSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SubcultureSystemStore { public: bool apply(const SubcultureSystemOp&); bool erase(std::uint64_t); const SubcultureSystemData* find(std::uint64_t) const; std::vector<SubcultureSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SubcultureSystemData> data_; };
}
