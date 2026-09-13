#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Plan heat sources, sinks, coolant loops, radiators, insulation, and emergency thermal shutdown.
struct HeatGridPlannerCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct HeatGridPlannerRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class HeatGridPlannerService {
public:
    bool submit(const HeatGridPlannerCommand& command);
    const HeatGridPlannerRecord* lookup(std::uint64_t subjectId) const;
    std::vector<HeatGridPlannerRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, HeatGridPlannerRecord> records_;
};

}
