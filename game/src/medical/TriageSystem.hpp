// Intended function: Prioritize casualties by airway, bleeding, shock, pain, infection risk, mobility, and available treatment capacity.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::medical {
struct TriageRecord {
    std::uint64_t patientId{};
    double priority{};
    double bleeding{};
    double shock{};
    double infectionRisk{};
    double mobility{};
};
class TriageRecordStore {
public:
 bool put(TriageRecord v); bool erase(std::uint64_t id);
 [[nodiscard]] const TriageRecord* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<TriageRecord>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const TriageRecord& v) noexcept; std::vector<TriageRecord> values_;
};
} // namespace elysium::medical
