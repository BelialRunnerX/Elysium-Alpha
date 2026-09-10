#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Expose read-only stable-ID inspection snapshots for entities, jobs, items, machines, settlements, and strategic records.
struct SimulationInspectorCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct SimulationInspectorRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class SimulationInspectorService {
public:
    bool submit(const SimulationInspectorCommand& command);
    const SimulationInspectorRecord* lookup(std::uint64_t subjectId) const;
    std::vector<SimulationInspectorRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, SimulationInspectorRecord> records_;
};

}
