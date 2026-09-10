#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Aggregate hunger, thirst, rest, safety, belonging, purpose, comfort, recreation, autonomy, and status needs.
struct NeedSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct NeedSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class NeedSystemStore { public: bool apply(const NeedSystemOp&); bool erase(std::uint64_t); const NeedSystemData* find(std::uint64_t) const; std::vector<NeedSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,NeedSystemData> data_; };
}
