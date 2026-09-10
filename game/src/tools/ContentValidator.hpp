// Intended function: Validate content IDs, namespaces, references, recipes, loot tables, structures, factions, localization, and mod compatibility.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::tools {
struct ContentValidationRecord {
    std::uint64_t recordId{};
    std::uint64_t contentId{};
    std::uint64_t category{};
    double severity{};
    std::uint64_t reasonCode{};
    std::uint64_t flags{};
};
class ContentValidationRecordIndex {
public:
 bool upsert(ContentValidationRecord value); bool erase(std::uint64_t id); [[nodiscard]] const ContentValidationRecord* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ContentValidationRecord>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const ContentValidationRecord& value) noexcept; std::vector<ContentValidationRecord> rows_;
};
}
