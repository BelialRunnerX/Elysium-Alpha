#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Plan sensors, relays, terminals, automation links, and bandwidth priorities across local infrastructure.
struct DataGridPlannerCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct DataGridPlannerRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class DataGridPlannerService {
public:
    bool submit(const DataGridPlannerCommand& command);
    const DataGridPlannerRecord* lookup(std::uint64_t subjectId) const;
    std::vector<DataGridPlannerRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, DataGridPlannerRecord> records_;
};

}
