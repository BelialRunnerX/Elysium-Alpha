#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent curfews, exceptions, enforcement zones, emergency lockdowns, and access-policy intents.
struct CurfewSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CurfewSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CurfewSystemStore { public: bool apply(const CurfewSystemOp&); bool erase(std::uint64_t); const CurfewSystemData* find(std::uint64_t) const; std::vector<CurfewSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CurfewSystemData> data_; };
}
