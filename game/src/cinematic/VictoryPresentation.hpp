#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build renderer/audio/UI-neutral presentation records for major victories, discoveries, disasters, and campaign milestones.
struct VictoryPresentationOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct VictoryPresentationData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class VictoryPresentationStore { public: bool apply(const VictoryPresentationOp&); bool erase(std::uint64_t); const VictoryPresentationData* find(std::uint64_t) const; std::vector<VictoryPresentationData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,VictoryPresentationData> data_; };
}
