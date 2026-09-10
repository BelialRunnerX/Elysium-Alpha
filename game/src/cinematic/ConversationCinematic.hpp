#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build presentation cues for dialogue scenes from participants, relationship, location, mood, and authored beats.
struct ConversationCinematicOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ConversationCinematicData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ConversationCinematicStore { public: bool apply(const ConversationCinematicOp&); bool erase(std::uint64_t); const ConversationCinematicData* find(std::uint64_t) const; std::vector<ConversationCinematicData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ConversationCinematicData> data_; };
}
