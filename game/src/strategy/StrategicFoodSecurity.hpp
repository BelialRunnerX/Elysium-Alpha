#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track food production, reserves, imports, famine risk, logistics vulnerability, and emergency rationing at regional LOD.
struct StrategicFoodSecurityOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct StrategicFoodSecurityData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class StrategicFoodSecurityStore { public: bool apply(const StrategicFoodSecurityOp&); bool erase(std::uint64_t); const StrategicFoodSecurityData* find(std::uint64_t) const; std::vector<StrategicFoodSecurityData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StrategicFoodSecurityData> data_; };
}
