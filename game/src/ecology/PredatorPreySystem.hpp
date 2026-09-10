#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track aggregate predator/prey population pressures, migration, reproduction, starvation, and local actor promotion.
struct PredatorPreySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct PredatorPreySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class PredatorPreySystemStore { public: bool apply(const PredatorPreySystemOp&); bool erase(std::uint64_t); const PredatorPreySystemData* find(std::uint64_t) const; std::vector<PredatorPreySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PredatorPreySystemData> data_; };
}
