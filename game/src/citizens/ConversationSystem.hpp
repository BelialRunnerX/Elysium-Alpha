#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate social interaction intents from proximity, relationship, needs, culture, memories, and current events.
struct ConversationSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ConversationSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ConversationSystemStore { public: bool apply(const ConversationSystemOp&); bool erase(std::uint64_t); const ConversationSystemData* find(std::uint64_t) const; std::vector<ConversationSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ConversationSystemData> data_; };
}
