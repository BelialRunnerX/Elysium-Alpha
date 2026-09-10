#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track friendship, trust, rivalry, family, mentorship, loyalty, and grievance links between stable citizen identities.
struct RelationshipSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct RelationshipSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class RelationshipSystemStore { public: bool apply(const RelationshipSystemOp&); bool erase(std::uint64_t); const RelationshipSystemData* find(std::uint64_t) const; std::vector<RelationshipSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RelationshipSystemData> data_; };
}
