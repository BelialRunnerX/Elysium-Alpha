#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate plausible owners, era, purpose, disaster, abandonment, modifications, occupants, and artifact context for ruins.
struct RuinHistoryGeneratorOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct RuinHistoryGeneratorData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class RuinHistoryGeneratorStore { public: bool apply(const RuinHistoryGeneratorOp&); bool erase(std::uint64_t); const RuinHistoryGeneratorData* find(std::uint64_t) const; std::vector<RuinHistoryGeneratorData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,RuinHistoryGeneratorData> data_; };
}
