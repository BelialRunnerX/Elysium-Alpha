#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent fiber/resin/composite layup, curing, quality, structural parts, armor, ship components, and repair patches.
struct CompositeIndustryOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CompositeIndustryData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CompositeIndustryStore { public: bool apply(const CompositeIndustryOp&); bool erase(std::uint64_t); const CompositeIndustryData* find(std::uint64_t) const; std::vector<CompositeIndustryData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CompositeIndustryData> data_; };
}
