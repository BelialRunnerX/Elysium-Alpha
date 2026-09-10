#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track local natural/artificial light summaries for farming, stealth, citizen needs, wildlife, and presentation.
struct LightEnvironmentOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct LightEnvironmentData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class LightEnvironmentStore { public: bool apply(const LightEnvironmentOp&); bool erase(std::uint64_t); const LightEnvironmentData* find(std::uint64_t) const; std::vector<LightEnvironmentData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,LightEnvironmentData> data_; };
}
