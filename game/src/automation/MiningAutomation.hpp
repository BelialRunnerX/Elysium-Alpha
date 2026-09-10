#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Coordinate extractor targets, drill wear, haul demand, and depletion-aware relocation requests.
struct MiningAutomationCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct MiningAutomationRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class MiningAutomationService {
public:
    bool submit(const MiningAutomationCommand& command);
    const MiningAutomationRecord* lookup(std::uint64_t subjectId) const;
    std::vector<MiningAutomationRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, MiningAutomationRecord> records_;
};

}
