// Intended function: Provide stable structure/blueprint definitions for habitat, industry, defense, utilities, civic, and orbital construction.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::content {
struct StructureRecord {
    std::uint64_t structureId{};
    std::uint64_t category{};
    std::uint64_t footprintHash{};
    std::uint64_t materialCostHash{};
    std::uint64_t powerDemand{};
    std::uint64_t flags{};
};
class StructureRecordRegistry {
public:
    bool publish(StructureRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const StructureRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<StructureRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const StructureRecord& r) noexcept;
    std::vector<StructureRecord> records_;
};
} // namespace elysium::content
