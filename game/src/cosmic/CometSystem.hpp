#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track deterministic comet orbits, volatile composition, observation opportunities, mining, and hazard windows.
struct CometSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CometSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CometSystemStore { public: bool apply(const CometSystemOp&); bool erase(std::uint64_t); const CometSystemData* find(std::uint64_t) const; std::vector<CometSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CometSystemData> data_; };
}
