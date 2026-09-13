// Intended function: Translate committed edits, combat, trade, deaths, discoveries, migrations, and sieges into cross-system stable events.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::simulation {
struct WorldEventRecord {
    std::uint64_t eventId{};
    std::uint64_t eventType{};
    std::uint64_t sourceId{};
    std::uint64_t targetId{};
    std::uint64_t siteId{};
    std::uint64_t tick{};
};
class WorldEventRecordTable {
public:
 bool set(WorldEventRecord value); bool remove(std::uint64_t id);
 [[nodiscard]] const WorldEventRecord* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<WorldEventRecord> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const WorldEventRecord& value) noexcept; std::vector<WorldEventRecord> rows_;
};
}
