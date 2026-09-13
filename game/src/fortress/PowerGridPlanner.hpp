#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Plan local power-grid expansion, redundancy, generation, storage, and load shedding from settlement demand.
struct PowerGridPlannerCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct PowerGridPlannerRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class PowerGridPlannerService {
public:
    bool submit(const PowerGridPlannerCommand& command);
    const PowerGridPlannerRecord* lookup(std::uint64_t subjectId) const;
    std::vector<PowerGridPlannerRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, PowerGridPlannerRecord> records_;
};

}
