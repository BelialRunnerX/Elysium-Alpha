#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate district adjacency, roads, utility spines, landmarks, zoning, and expansion seeds from settlement context.
struct SettlementDistrictGeneratorOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SettlementDistrictGeneratorData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SettlementDistrictGeneratorStore { public: bool apply(const SettlementDistrictGeneratorOp&); bool erase(std::uint64_t); const SettlementDistrictGeneratorData* find(std::uint64_t) const; std::vector<SettlementDistrictGeneratorData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SettlementDistrictGeneratorData> data_; };
}
