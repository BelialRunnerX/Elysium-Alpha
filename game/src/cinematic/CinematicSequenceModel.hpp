#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent camera, actor, subtitle, audio, event, skip, branch, and timing cues without owning simulation state.
struct CinematicSequenceModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CinematicSequenceModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CinematicSequenceModelStore { public: bool apply(const CinematicSequenceModelOp&); bool erase(std::uint64_t); const CinematicSequenceModelData* find(std::uint64_t) const; std::vector<CinematicSequenceModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CinematicSequenceModelData> data_; };
}
