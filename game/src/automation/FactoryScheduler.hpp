#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Schedule machine work batches from recipe demand, buffers, power, maintenance, and logistics availability.
struct FactorySchedulerCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct FactorySchedulerRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class FactorySchedulerService {
public:
    bool submit(const FactorySchedulerCommand& command);
    const FactorySchedulerRecord* lookup(std::uint64_t subjectId) const;
    std::vector<FactorySchedulerRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, FactorySchedulerRecord> records_;
};

}
