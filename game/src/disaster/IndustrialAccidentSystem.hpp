#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate explosions, toxic releases, fires, pressure failures, contamination, casualties, investigation, and shutdowns.
struct IndustrialAccidentSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct IndustrialAccidentSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class IndustrialAccidentSystemStore { public: bool apply(const IndustrialAccidentSystemOp&); bool erase(std::uint64_t); const IndustrialAccidentSystemData* find(std::uint64_t) const; std::vector<IndustrialAccidentSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,IndustrialAccidentSystemData> data_; };
}
