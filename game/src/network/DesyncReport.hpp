#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Capture future deterministic divergence metadata, stable object hashes, command windows, and subsystem diagnostics.
struct DesyncReportOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct DesyncReportData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class DesyncReportStore { public: bool apply(const DesyncReportOp&); bool erase(std::uint64_t); const DesyncReportData* find(std::uint64_t) const; std::vector<DesyncReportData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,DesyncReportData> data_; };
}
