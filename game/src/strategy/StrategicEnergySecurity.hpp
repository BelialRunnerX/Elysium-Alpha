#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track fuel, power resources, generation capacity, imports, reserves, and infrastructure vulnerability.
struct StrategicEnergySecurityOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct StrategicEnergySecurityData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class StrategicEnergySecurityStore { public: bool apply(const StrategicEnergySecurityOp&); bool erase(std::uint64_t); const StrategicEnergySecurityData* find(std::uint64_t) const; std::vector<StrategicEnergySecurityData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,StrategicEnergySecurityData> data_; };
}
