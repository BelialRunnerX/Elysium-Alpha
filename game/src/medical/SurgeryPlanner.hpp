// Intended function: Plan staged surgery with diagnosis, procedure prerequisites, surgeon skill, tools, anesthesia, blood, and recovery.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::medical {
struct SurgeryPlan {
    std::uint64_t planId{};
    std::uint64_t patientId{};
    std::uint64_t procedureId{};
    double readiness{};
    double risk{};
    std::uint64_t state{};
};
class SurgeryPlanStore {
public:
 bool put(SurgeryPlan v); bool erase(std::uint64_t id);
 [[nodiscard]] const SurgeryPlan* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<SurgeryPlan>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const SurgeryPlan& v) noexcept; std::vector<SurgeryPlan> values_;
};
} // namespace elysium::medical
