#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Allocate ecosystem actor budgets and spawn/despawn pressure from local ecology summaries without breaking persistence.
struct WildlifeDirectorAIOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct WildlifeDirectorAIData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class WildlifeDirectorAIStore { public: bool apply(const WildlifeDirectorAIOp&); bool erase(std::uint64_t); const WildlifeDirectorAIData* find(std::uint64_t) const; std::vector<WildlifeDirectorAIData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,WildlifeDirectorAIData> data_; };
}
