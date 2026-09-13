#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track spread of panic, enthusiasm, anger, hope, protest, celebration, and risk behavior through social networks.
struct SocialContagionOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SocialContagionData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SocialContagionStore { public: bool apply(const SocialContagionOp&); bool erase(std::uint64_t); const SocialContagionData* find(std::uint64_t) const; std::vector<SocialContagionData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SocialContagionData> data_; };
}
