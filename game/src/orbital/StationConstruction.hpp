#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build orbital stations from stable module graphs, construction stages, utility requirements, and docking topology.
struct StationConstructionCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct StationConstructionRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class StationConstructionService {
public:
    bool submit(const StationConstructionCommand& command);
    const StationConstructionRecord* lookup(std::uint64_t subjectId) const;
    std::vector<StationConstructionRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, StationConstructionRecord> records_;
};

}
