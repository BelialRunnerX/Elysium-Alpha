#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track Imperial Court factions, interests, patrons, rivals, influence, agendas, and reactions to player/faction actions.
struct CourtFactionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CourtFactionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CourtFactionSystemStore { public: bool apply(const CourtFactionSystemOp&); bool erase(std::uint64_t); const CourtFactionSystemData* find(std::uint64_t) const; std::vector<CourtFactionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CourtFactionSystemData> data_; };
}
