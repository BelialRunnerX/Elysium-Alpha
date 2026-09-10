#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Aggregate frame/simulation subsystem costs against configurable budgets without modifying authoritative behavior.
struct PerformanceBudgetTrackerCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct PerformanceBudgetTrackerRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class PerformanceBudgetTrackerService {
public:
    bool submit(const PerformanceBudgetTrackerCommand& command);
    const PerformanceBudgetTrackerRecord* lookup(std::uint64_t subjectId) const;
    std::vector<PerformanceBudgetTrackerRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, PerformanceBudgetTrackerRecord> records_;
};

}
