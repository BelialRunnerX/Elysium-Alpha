#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track audience requests, preparation, attendees, petitions, negotiations, protocol, outcomes, and political consequences.
struct CourtAudienceSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CourtAudienceSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CourtAudienceSystemStore { public: bool apply(const CourtAudienceSystemOp&); bool erase(std::uint64_t); const CourtAudienceSystemData* find(std::uint64_t) const; std::vector<CourtAudienceSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CourtAudienceSystemData> data_; };
}
