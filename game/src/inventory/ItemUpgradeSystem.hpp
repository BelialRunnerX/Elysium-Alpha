#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Apply deterministic quality upgrades, socketing, reforging, tuning, and modification recipes to unique items.
struct ItemUpgradeSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct ItemUpgradeSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class ItemUpgradeSystemService {
public:
    bool submit(const ItemUpgradeSystemCommand& command);
    const ItemUpgradeSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<ItemUpgradeSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, ItemUpgradeSystemRecord> records_;
};

}
