#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build settlement needs, population, work, services, utilities, threats, policies, and alerts projections.
struct ColonyScreenModelCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct ColonyScreenModelRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class ColonyScreenModelService {
public:
    bool submit(const ColonyScreenModelCommand& command);
    const ColonyScreenModelRecord* lookup(std::uint64_t subjectId) const;
    std::vector<ColonyScreenModelRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, ColonyScreenModelRecord> records_;
};

}
