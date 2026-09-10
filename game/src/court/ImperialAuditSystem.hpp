#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track inspections of settlements, trade, research, military assets, registrations, taxes, and prohibited activities.
struct ImperialAuditSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ImperialAuditSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ImperialAuditSystemStore { public: bool apply(const ImperialAuditSystemOp&); bool erase(std::uint64_t); const ImperialAuditSystemData* find(std::uint64_t) const; std::vector<ImperialAuditSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ImperialAuditSystemData> data_; };
}
