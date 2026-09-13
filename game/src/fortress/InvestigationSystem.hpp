// Intended function: Track cases, evidence, witness statements, sensor records, suspects, confidence, investigator, and verdict readiness.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::fortress {
struct InvestigationCase {
    std::uint64_t caseId{};
    std::uint64_t incidentId{};
    std::uint64_t investigatorId{};
    double evidenceScore{};
    std::uint64_t suspectCount{};
    std::uint64_t state{};
};
class InvestigationCaseStore {
public:
 bool put(InvestigationCase v); bool erase(std::uint64_t id);
 [[nodiscard]] const InvestigationCase* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<InvestigationCase>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const InvestigationCase& v) noexcept; std::vector<InvestigationCase> values_;
};
} // namespace elysium::fortress
