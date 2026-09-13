#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track vaccine research, production, distribution, uptake, efficacy, strain coverage, and shortages.
struct VaccineProgramOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct VaccineProgramData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class VaccineProgramStore { public: bool apply(const VaccineProgramOp&); bool erase(std::uint64_t); const VaccineProgramData* find(std::uint64_t) const; std::vector<VaccineProgramData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,VaccineProgramData> data_; };
}
