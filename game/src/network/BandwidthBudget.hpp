#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Allocate future replication bandwidth among critical gameplay, nearby actors, UI inspection, chat, and presentation channels.
struct BandwidthBudgetOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct BandwidthBudgetData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class BandwidthBudgetStore { public: bool apply(const BandwidthBudgetOp&); bool erase(std::uint64_t); const BandwidthBudgetData* find(std::uint64_t) const; std::vector<BandwidthBudgetData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BandwidthBudgetData> data_; };
}
