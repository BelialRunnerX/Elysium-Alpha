#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track bounded rumors with source, subject, confidence, distortion, propagation, correction, and social consequences.
struct RumorSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct RumorSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class RumorSystemStore { public: bool apply(const RumorSystemOp&); bool erase(std::uint64_t); const RumorSystemData* find(std::uint64_t) const; std::vector<RumorSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RumorSystemData> data_; };
}
