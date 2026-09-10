#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track settlement-level public funds, earmarks, obligations, taxes, wages, procurement, and emergency reserves.
struct TreasurySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct TreasurySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class TreasurySystemStore { public: bool apply(const TreasurySystemOp&); bool erase(std::uint64_t); const TreasurySystemData* find(std::uint64_t) const; std::vector<TreasurySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,TreasurySystemData> data_; };
}
