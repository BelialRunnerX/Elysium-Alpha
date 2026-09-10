#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track campaign-scale mystery threads, evidence, hypotheses, sites, contradictions, revelations, and endgame gates.
struct GalacticMysterySystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct GalacticMysterySystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class GalacticMysterySystemStore { public: bool apply(const GalacticMysterySystemOp&); bool erase(std::uint64_t); const GalacticMysterySystemData* find(std::uint64_t) const; std::vector<GalacticMysterySystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,GalacticMysterySystemData> data_; };
}
