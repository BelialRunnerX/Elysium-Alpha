#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent ceramic powders, firing, refractory components, armor, insulation, electronics substrates, and construction products.
struct CeramicsIndustryOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CeramicsIndustryData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CeramicsIndustryStore { public: bool apply(const CeramicsIndustryOp&); bool erase(std::uint64_t); const CeramicsIndustryData* find(std::uint64_t) const; std::vector<CeramicsIndustryData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CeramicsIndustryData> data_; };
}
