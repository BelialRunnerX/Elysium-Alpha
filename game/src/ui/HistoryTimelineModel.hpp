#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build zoomable chronological projections linking people, settlements, factions, battles, discoveries, artifacts, and disasters.
struct HistoryTimelineModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct HistoryTimelineModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class HistoryTimelineModelStore { public: bool apply(const HistoryTimelineModelOp&); bool erase(std::uint64_t); const HistoryTimelineModelData* find(std::uint64_t) const; std::vector<HistoryTimelineModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,HistoryTimelineModelData> data_; };
}
