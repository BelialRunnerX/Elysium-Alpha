#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track organizations through founding, leaders, reforms, splits, scandals, achievements, relocations, and dissolution.
struct InstitutionHistorySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct InstitutionHistorySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class InstitutionHistorySystemStore { public: bool apply(const InstitutionHistorySystemOp&); bool erase(std::uint64_t); const InstitutionHistorySystemData* find(std::uint64_t) const; std::vector<InstitutionHistorySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,InstitutionHistorySystemData> data_; };
}
