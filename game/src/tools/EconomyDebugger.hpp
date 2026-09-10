#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Expose market supply/demand, price factors, trade routes, shortages, contracts, and ownership-transfer diagnostics.
struct EconomyDebuggerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct EconomyDebuggerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class EconomyDebuggerStore { public: bool apply(const EconomyDebuggerOp&); bool erase(std::uint64_t); const EconomyDebuggerData* find(std::uint64_t) const; std::vector<EconomyDebuggerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,EconomyDebuggerData> data_; };
}
