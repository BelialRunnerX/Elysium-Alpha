// Intended function: Provide deterministic weighted loot-table entries with tier gates, quantity ranges, uniqueness, and provenance policies.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::content {
struct LootTableEntry {
    std::uint64_t entryId{};
    std::uint64_t tableId{};
    std::uint64_t contentId{};
    double weight{};
    std::uint64_t minCount{};
    std::uint64_t maxCount{};
};
class LootTableEntryRegistry {
public:
    bool publish(LootTableEntry record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const LootTableEntry* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<LootTableEntry>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const LootTableEntry& r) noexcept;
    std::vector<LootTableEntry> records_;
};
} // namespace elysium::content
