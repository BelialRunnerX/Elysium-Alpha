#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Expose Chronicle records, cross-links, provenance, relationship memories, and generated narrative inputs.
struct HistoryDebuggerOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct HistoryDebuggerData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class HistoryDebuggerStore { public: bool apply(const HistoryDebuggerOp&); bool erase(std::uint64_t); const HistoryDebuggerData* find(std::uint64_t) const; std::vector<HistoryDebuggerData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,HistoryDebuggerData> data_; };
}
