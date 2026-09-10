#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Plan waste collection, recycling, hazardous isolation, composting, and disposal workflows.
struct WasteProcessingPlannerCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct WasteProcessingPlannerRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class WasteProcessingPlannerService {
public:
    bool submit(const WasteProcessingPlannerCommand& command);
    const WasteProcessingPlannerRecord* lookup(std::uint64_t subjectId) const;
    std::vector<WasteProcessingPlannerRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, WasteProcessingPlannerRecord> records_;
};

}
