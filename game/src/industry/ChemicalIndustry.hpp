#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent acids, solvents, polymers, fertilizers, medicines, propellants, toxins, and waste-processing production chains.
struct ChemicalIndustryOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ChemicalIndustryData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ChemicalIndustryStore { public: bool apply(const ChemicalIndustryOp&); bool erase(std::uint64_t); const ChemicalIndustryData* find(std::uint64_t) const; std::vector<ChemicalIndustryData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ChemicalIndustryData> data_; };
}
