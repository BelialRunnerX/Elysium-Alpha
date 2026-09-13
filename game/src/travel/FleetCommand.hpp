#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent fleet formations, roles, objectives, engagement policies, escorts, and route commands using stable ship IDs.
struct FleetCommandCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct FleetCommandRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class FleetCommandService {
public:
    bool submit(const FleetCommandCommand& command);
    const FleetCommandRecord* lookup(std::uint64_t subjectId) const;
    std::vector<FleetCommandRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, FleetCommandRecord> records_;
};

}
