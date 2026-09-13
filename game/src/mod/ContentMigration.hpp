// Intended function: Represent stable content-ID migration maps for renamed/removed/replaced blocks, items, recipes, factions, abilities, and structures.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::mod {
struct ContentMigrationRecord {
    std::uint64_t migrationId{};
    std::uint64_t oldContentId{};
    std::uint64_t newContentId{};
    std::uint64_t minSchema{};
    std::uint64_t maxSchema{};
    std::uint64_t flags{};
};
class ContentMigrationRecordCollection {
public:
 bool store(ContentMigrationRecord value); bool erase(std::uint64_t id); [[nodiscard]] const ContentMigrationRecord* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ContentMigrationRecord>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const ContentMigrationRecord& v) noexcept; std::vector<ContentMigrationRecord> rows_;
};
}
