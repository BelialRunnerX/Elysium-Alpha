// Intended function: Capture bounded per-tick simulation phase timings, decisions, commands, conflicts, state revisions, and reason codes.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::tools {
struct SimulationTraceRecord {
    std::uint64_t recordId{};
    std::uint64_t tick{};
    std::uint64_t phase{};
    std::uint64_t systemId{};
    std::uint64_t commandCount{};
    std::uint64_t conflictCount{};
};
class SimulationTraceRecordIndex {
public:
 bool upsert(SimulationTraceRecord value); bool erase(std::uint64_t id); [[nodiscard]] const SimulationTraceRecord* find(std::uint64_t id) const; [[nodiscard]] const std::vector<SimulationTraceRecord>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const SimulationTraceRecord& value) noexcept; std::vector<SimulationTraceRecord> rows_;
};
}
