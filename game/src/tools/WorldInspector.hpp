// Intended function: Expose stable read-only world/entity/object snapshots and reason strings for developer inspection tooling.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::tools {
struct InspectionRecord {
    std::uint64_t recordId{};
    std::uint64_t stableId{};
    std::uint64_t kind{};
    std::uint64_t stateHash{};
    std::uint64_t reasonCode{};
    std::uint64_t tick{};
};
class InspectionRecordRegistry {
public:
    bool publish(InspectionRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const InspectionRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<InspectionRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const InspectionRecord& r) noexcept;
    std::vector<InspectionRecord> records_;
};
} // namespace elysium::tools
