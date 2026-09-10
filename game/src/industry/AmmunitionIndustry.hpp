#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent projectile, propellant, casing, energetic, missile, battery, and specialized ammunition production.
struct AmmunitionIndustryOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct AmmunitionIndustryData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class AmmunitionIndustryStore { public: bool apply(const AmmunitionIndustryOp&); bool erase(std::uint64_t); const AmmunitionIndustryData* find(std::uint64_t) const; std::vector<AmmunitionIndustryData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,AmmunitionIndustryData> data_; };
}
