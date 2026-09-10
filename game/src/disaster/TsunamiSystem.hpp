#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate wave impacts from seismic/volcanic/impact triggers with coastal flooding, debris, evacuation, and damage.
struct TsunamiSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct TsunamiSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class TsunamiSystemStore { public: bool apply(const TsunamiSystemOp&); bool erase(std::uint64_t); const TsunamiSystemData* find(std::uint64_t) const; std::vector<TsunamiSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,TsunamiSystemData> data_; };
}
