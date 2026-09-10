#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track suspicious activity, investigations, surveillance warrants, deception, moles, and security posture.
struct CounterintelligenceSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CounterintelligenceSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CounterintelligenceSystemStore { public: bool apply(const CounterintelligenceSystemOp&); bool erase(std::uint64_t); const CounterintelligenceSystemData* find(std::uint64_t) const; std::vector<CounterintelligenceSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CounterintelligenceSystemData> data_; };
}
