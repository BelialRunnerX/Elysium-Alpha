#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent profession preferences, certifications, uniforms, tools, role priorities, and career history.
struct ProfessionSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct ProfessionSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class ProfessionSystemStore { public: bool apply(const ProfessionSystemOp&); bool erase(std::uint64_t); const ProfessionSystemData* find(std::uint64_t) const; std::vector<ProfessionSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,ProfessionSystemData> data_; };
}
