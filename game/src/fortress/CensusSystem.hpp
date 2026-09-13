#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Maintain bounded settlement demographic summaries by species, age cohort, profession, household, citizenship, and status.
struct CensusSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CensusSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CensusSystemStore { public: bool apply(const CensusSystemOp&); bool erase(std::uint64_t); const CensusSystemData* find(std::uint64_t) const; std::vector<CensusSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CensusSystemData> data_; };
}
