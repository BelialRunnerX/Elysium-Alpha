#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Resolve weighted loot pools from source type, biome, faction, danger, rarity, and player progression context.
struct LootTableResolverCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct LootTableResolverRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class LootTableResolverService {
public:
    bool submit(const LootTableResolverCommand& command);
    const LootTableResolverRecord* lookup(std::uint64_t subjectId) const;
    std::vector<LootTableResolverRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, LootTableResolverRecord> records_;
};

}
