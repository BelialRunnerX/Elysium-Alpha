// Intended function: Assign stable citizens to jobs using labor permissions, skill, distance, urgency, equipment, and deterministic tie-breaks.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::fortress {
struct WorkAssignment {
    std::uint64_t assignmentId{};
    std::uint64_t citizenId{};
    std::uint64_t jobId{};
    double score{};
    double priority{};
    std::uint64_t state{};
};
class WorkAssignmentStore {
public:
 bool put(WorkAssignment v); bool erase(std::uint64_t id);
 [[nodiscard]] const WorkAssignment* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<WorkAssignment>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const WorkAssignment& v) noexcept; std::vector<WorkAssignment> values_;
};
} // namespace elysium::fortress
