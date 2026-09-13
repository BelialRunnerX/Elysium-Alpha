#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate stock transfer and replenishment intents from min/max policies and bounded storage summaries.
struct WarehouseAutomationCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct WarehouseAutomationRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class WarehouseAutomationService {
public:
    bool submit(const WarehouseAutomationCommand& command);
    const WarehouseAutomationRecord* lookup(std::uint64_t subjectId) const;
    std::vector<WarehouseAutomationRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, WarehouseAutomationRecord> records_;
};

}
