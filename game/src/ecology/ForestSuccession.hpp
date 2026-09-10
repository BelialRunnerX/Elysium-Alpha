#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track vegetation succession, biomass, fire history, disturbance, harvesting, regeneration, and habitat structure.
struct ForestSuccessionOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ForestSuccessionData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ForestSuccessionStore { public: bool apply(const ForestSuccessionOp&); bool erase(std::uint64_t); const ForestSuccessionData* find(std::uint64_t) const; std::vector<ForestSuccessionData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ForestSuccessionData> data_; };
}
