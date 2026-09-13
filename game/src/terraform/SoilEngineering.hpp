#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track soil chemistry, fertility, contamination removal, microbiome seeding, and agricultural conversion.
struct SoilEngineeringOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SoilEngineeringData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SoilEngineeringStore { public: bool apply(const SoilEngineeringOp&); bool erase(std::uint64_t); const SoilEngineeringData* find(std::uint64_t) const; std::vector<SoilEngineeringData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SoilEngineeringData> data_; };
}
