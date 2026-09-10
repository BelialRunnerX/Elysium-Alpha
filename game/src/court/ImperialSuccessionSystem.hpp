#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track succession candidates, legitimacy, support blocs, crises, regencies, and campaign-scale consequences.
struct ImperialSuccessionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ImperialSuccessionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ImperialSuccessionSystemStore { public: bool apply(const ImperialSuccessionSystemOp&); bool erase(std::uint64_t); const ImperialSuccessionSystemData* find(std::uint64_t) const; std::vector<ImperialSuccessionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ImperialSuccessionSystemData> data_; };
}
