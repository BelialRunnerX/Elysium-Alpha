#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Expose AI goals, utilities, plans, memories, path requests, blockers, and recent decisions for inspection.
struct AIDebuggerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct AIDebuggerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class AIDebuggerStore { public: bool apply(const AIDebuggerOp&); bool erase(std::uint64_t); const AIDebuggerData* find(std::uint64_t) const; std::vector<AIDebuggerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,AIDebuggerData> data_; };
}
