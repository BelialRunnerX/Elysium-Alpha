#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track construction, trade, research, weapons, docking, mining, and hazardous-work permits by stable subject.
struct PermitSystemOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct PermitSystemData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class PermitSystemStore { public: bool apply(const PermitSystemOp&); bool erase(std::uint64_t); const PermitSystemData* find(std::uint64_t) const; std::vector<PermitSystemData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PermitSystemData> data_; };
}
