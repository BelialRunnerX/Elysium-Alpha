#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Advance citizen skills from practice, teaching, aptitude, fatigue, equipment, and diminishing returns.
struct SkillProgressionOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SkillProgressionData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SkillProgressionStore { public: bool apply(const SkillProgressionOp&); bool erase(std::uint64_t); const SkillProgressionData* find(std::uint64_t) const; std::vector<SkillProgressionData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SkillProgressionData> data_; };
}
