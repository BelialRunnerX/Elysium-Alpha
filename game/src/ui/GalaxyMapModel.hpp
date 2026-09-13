#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build discovered-system, route, faction, signal, hazard, mission, and fleet projections for the galaxy map.
struct GalaxyMapModelCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct GalaxyMapModelRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class GalaxyMapModelService {
public:
    bool submit(const GalaxyMapModelCommand& command);
    const GalaxyMapModelRecord* lookup(std::uint64_t subjectId) const;
    std::vector<GalaxyMapModelRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, GalaxyMapModelRecord> records_;
};

}
