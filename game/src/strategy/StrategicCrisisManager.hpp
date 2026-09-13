#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate response priorities for famine, invasion, rebellion, plague, disaster, blockade, and infrastructure collapse.
struct StrategicCrisisManagerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct StrategicCrisisManagerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class StrategicCrisisManagerStore { public: bool apply(const StrategicCrisisManagerOp&); bool erase(std::uint64_t); const StrategicCrisisManagerData* find(std::uint64_t) const; std::vector<StrategicCrisisManagerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StrategicCrisisManagerData> data_; };
}
