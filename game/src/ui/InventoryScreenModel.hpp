#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build inventory/filter/equipment/container view models and stable command intents for player-facing UI.
struct InventoryScreenModelCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct InventoryScreenModelRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class InventoryScreenModelService {
public:
    bool submit(const InventoryScreenModelCommand& command);
    const InventoryScreenModelRecord* lookup(std::uint64_t subjectId) const;
    std::vector<InventoryScreenModelRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, InventoryScreenModelRecord> records_;
};

}
