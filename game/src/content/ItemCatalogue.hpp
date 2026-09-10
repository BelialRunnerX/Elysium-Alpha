// Intended function: Provide stable item definitions for resources, tools, gear, consumables, components, artifacts, and logistics policy.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::content {
struct ItemRecord {
    std::uint64_t itemId{};
    std::uint64_t category{};
    std::uint64_t materialId{};
    double baseValue{};
    double mass{};
    std::uint64_t flags{};
};
class ItemRecordRegistry {
public:
    bool publish(ItemRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const ItemRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<ItemRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const ItemRecord& r) noexcept;
    std::vector<ItemRecord> records_;
};
} // namespace elysium::content
