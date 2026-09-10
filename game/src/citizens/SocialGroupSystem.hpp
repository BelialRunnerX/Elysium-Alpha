#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track clubs, work crews, ideological circles, research teams, gangs, and informal social communities.
struct SocialGroupSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SocialGroupSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SocialGroupSystemStore { public: bool apply(const SocialGroupSystemOp&); bool erase(std::uint64_t); const SocialGroupSystemData* find(std::uint64_t) const; std::vector<SocialGroupSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SocialGroupSystemData> data_; };
}
