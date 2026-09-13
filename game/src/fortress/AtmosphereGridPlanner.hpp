#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Plan vents, pumps, scrubbers, pressure zones, and emergency isolation around bounded room graphs.
struct AtmosphereGridPlannerCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct AtmosphereGridPlannerRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class AtmosphereGridPlannerService {
public:
    bool submit(const AtmosphereGridPlannerCommand& command);
    const AtmosphereGridPlannerRecord* lookup(std::uint64_t subjectId) const;
    std::vector<AtmosphereGridPlannerRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, AtmosphereGridPlannerRecord> records_;
};

}
