// Intended function: Inspect versioned save sections, stable IDs, generator fingerprints, checksums, tombstones, migrations, and dependency closure.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::tools {
struct SaveInspectionRecord {
    std::uint64_t recordId{};
    std::uint64_t sectionId{};
    std::uint64_t stableId{};
    std::uint64_t schema{};
    std::uint64_t status{};
    std::uint64_t flags{};
};
class SaveInspectionRecordIndex {
public:
 bool upsert(SaveInspectionRecord value); bool erase(std::uint64_t id); [[nodiscard]] const SaveInspectionRecord* find(std::uint64_t id) const; [[nodiscard]] const std::vector<SaveInspectionRecord>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const SaveInspectionRecord& value) noexcept; std::vector<SaveInspectionRecord> rows_;
};
}
