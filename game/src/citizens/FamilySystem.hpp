#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track parentage, partners, kinship, births, inheritance hooks, guardianship, and family history.
struct FamilySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct FamilySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class FamilySystemStore { public: bool apply(const FamilySystemOp&); bool erase(std::uint64_t); const FamilySystemData* find(std::uint64_t) const; std::vector<FamilySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,FamilySystemData> data_; };
}
