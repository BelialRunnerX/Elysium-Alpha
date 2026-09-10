#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Schedule roads, transit, utilities, defenses, parks, monuments, housing, and district upgrades.
struct PublicWorksOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct PublicWorksData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class PublicWorksStore { public: bool apply(const PublicWorksOp&); bool erase(std::uint64_t); const PublicWorksData* find(std::uint64_t) const; std::vector<PublicWorksData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PublicWorksData> data_; };
}
