#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Advance projectile state using deterministic kinematics, collision queries, lifetime, penetration, and impact events.
struct ProjectileSimulationCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct ProjectileSimulationRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class ProjectileSimulationService {
public:
    bool submit(const ProjectileSimulationCommand& command);
    const ProjectileSimulationRecord* lookup(std::uint64_t subjectId) const;
    std::vector<ProjectileSimulationRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, ProjectileSimulationRecord> records_;
};

}
