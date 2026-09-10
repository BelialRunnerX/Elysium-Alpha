#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Generate and track unexplained signals with direction, age, confidence, decoding, investigation, and story hooks.
struct DeepSpaceSignalSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct DeepSpaceSignalSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class DeepSpaceSignalSystemStore { public: bool apply(const DeepSpaceSignalSystemOp&); bool erase(std::uint64_t); const DeepSpaceSignalSystemData* find(std::uint64_t) const; std::vector<DeepSpaceSignalSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DeepSpaceSignalSystemData> data_; };
}
