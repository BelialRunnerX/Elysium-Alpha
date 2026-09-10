#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track ideological disagreements, polarization, persuasion, faction formation, compromise, and violence risk.
struct IdeologyConflictOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct IdeologyConflictData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class IdeologyConflictStore { public: bool apply(const IdeologyConflictOp&); bool erase(std::uint64_t); const IdeologyConflictData* find(std::uint64_t) const; std::vector<IdeologyConflictData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,IdeologyConflictData> data_; };
}
