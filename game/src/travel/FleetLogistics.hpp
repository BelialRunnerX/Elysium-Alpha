#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Aggregate fleet fuel, ammunition, repair, crew, cargo, and supply endurance for strategic planning.
struct FleetLogisticsCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct FleetLogisticsRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class FleetLogisticsService {
public:
    bool submit(const FleetLogisticsCommand& command);
    const FleetLogisticsRecord* lookup(std::uint64_t subjectId) const;
    std::vector<FleetLogisticsRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, FleetLogisticsRecord> records_;
};

}
