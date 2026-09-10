#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate Court-specific commissions with prestige, secrecy, deadlines, faction sponsors, rewards, and failure consequences.
struct CourtCommissionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CourtCommissionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CourtCommissionSystemStore { public: bool apply(const CourtCommissionSystemOp&); bool erase(std::uint64_t); const CourtCommissionSystemData* find(std::uint64_t) const; std::vector<CourtCommissionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CourtCommissionSystemData> data_; };
}
