// Intended function: Provide authored creature definitions layered over procedural body plans, factions, abilities, loot, and habitat tags.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::content {
struct CreatureRecord {
    std::uint64_t creatureId{};
    std::uint64_t bodyPlanId{};
    std::uint64_t factionId{};
    std::uint64_t abilitySetId{};
    std::uint64_t lootTableId{};
    std::uint64_t habitatTag{};
};
class CreatureRecordRegistry {
public:
    bool publish(CreatureRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const CreatureRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<CreatureRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const CreatureRecord& r) noexcept;
    std::vector<CreatureRecord> records_;
};
} // namespace elysium::content
