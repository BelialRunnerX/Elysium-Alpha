// Intended function: Generate compact pre-simulation history seeds for founding, migration, wars, sites, artifacts, rulers, and institutional changes.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::procedural {
struct HistorySeedRecord {
    std::uint64_t eventId{};
    std::uint64_t seed{};
    std::uint64_t regionId{};
    std::uint64_t actorId{};
    std::uint64_t subjectId{};
    std::uint64_t epoch{};
};
class HistorySeedRecordTable {
public:
 bool set(HistorySeedRecord value); bool remove(std::uint64_t id);
 [[nodiscard]] const HistorySeedRecord* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<HistorySeedRecord> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const HistorySeedRecord& value) noexcept; std::vector<HistorySeedRecord> rows_;
};
}
