// Intended function: Coordinate Sense, Plan, Resolve, Commit, Persist, and Present phase sequencing with deterministic tick identity.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::simulation {
struct PhaseRecord {
    std::uint64_t tick{};
    std::uint64_t phase{};
    std::uint64_t sequence{};
    double budget{};
    double consumed{};
    std::uint64_t flags{};
};
class PhaseRecordTable {
public:
 bool set(PhaseRecord value); bool remove(std::uint64_t id);
 [[nodiscard]] const PhaseRecord* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<PhaseRecord> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const PhaseRecord& value) noexcept; std::vector<PhaseRecord> rows_;
};
}
