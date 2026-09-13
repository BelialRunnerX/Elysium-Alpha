#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Select authored/procedural story opportunities from current world facts without forcing outcomes or mutating history directly.
struct NarrativeDirectorAIOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct NarrativeDirectorAIData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class NarrativeDirectorAIStore { public: bool apply(const NarrativeDirectorAIOp&); bool erase(std::uint64_t); const NarrativeDirectorAIData* find(std::uint64_t) const; std::vector<NarrativeDirectorAIData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,NarrativeDirectorAIData> data_; };
}
