#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Summarize schema, checksums, missing dependencies, tombstones, generator versions, and recovery candidates.
struct SaveIntegrityReportOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SaveIntegrityReportData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SaveIntegrityReportStore { public: bool apply(const SaveIntegrityReportOp&); bool erase(std::uint64_t); const SaveIntegrityReportData* find(std::uint64_t) const; std::vector<SaveIntegrityReportData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SaveIntegrityReportData> data_; };
}
