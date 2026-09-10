// Intended function: Provide stable ship hull definitions, module capacities, cargo limits, mass, thrust class, and progression tier.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::content {
struct ShipRecord {
    std::uint64_t shipId{};
    std::uint64_t hullClass{};
    std::uint64_t moduleSlots{};
    std::uint64_t cargoCapacity{};
    double mass{};
    std::uint64_t tier{};
};
class ShipRecordRegistry {
public:
    bool publish(ShipRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const ShipRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<ShipRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const ShipRecord& r) noexcept;
    std::vector<ShipRecord> records_;
};
} // namespace elysium::content
