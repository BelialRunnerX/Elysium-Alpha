#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Merge sensor, scout, spy, rumor, trade, and historical reports into confidence-scored strategic facts.
struct IntelFusionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct IntelFusionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class IntelFusionSystemStore { public: bool apply(const IntelFusionSystemOp&); bool erase(std::uint64_t); const IntelFusionSystemData* find(std::uint64_t) const; std::vector<IntelFusionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,IntelFusionSystemData> data_; };
}
