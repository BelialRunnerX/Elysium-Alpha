#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track significant natural/industrial disasters, warnings, casualties, responses, reconstruction, and political consequences.
struct DisasterHistorySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct DisasterHistorySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class DisasterHistorySystemStore { public: bool apply(const DisasterHistorySystemOp&); bool erase(std::uint64_t); const DisasterHistorySystemData* find(std::uint64_t) const; std::vector<DisasterHistorySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DisasterHistorySystemData> data_; };
}
