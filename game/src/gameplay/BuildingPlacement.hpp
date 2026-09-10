#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Validate construction placement against address ownership, terrain, support, clearance, claims, and utility access.
struct BuildingPlacementCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct BuildingPlacementRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class BuildingPlacementService {
public:
    bool submit(const BuildingPlacementCommand& command);
    const BuildingPlacementRecord* lookup(std::uint64_t subjectId) const;
    std::vector<BuildingPlacementRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, BuildingPlacementRecord> records_;
};

}
