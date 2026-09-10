// Intended function: Sample deterministic galaxy/planet/chunk/worldgen inputs and outputs with named seed streams and compatibility fingerprints.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::tools {
struct WorldgenInspectionRecord {
    std::uint64_t recordId{};
    std::uint64_t seed{};
    std::uint64_t generatorId{};
    std::uint64_t inputHash{};
    std::uint64_t outputHash{};
    std::uint64_t flags{};
};
class WorldgenInspectionRecordIndex {
public:
 bool upsert(WorldgenInspectionRecord value); bool erase(std::uint64_t id); [[nodiscard]] const WorldgenInspectionRecord* find(std::uint64_t id) const; [[nodiscard]] const std::vector<WorldgenInspectionRecord>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const WorldgenInspectionRecord& value) noexcept; std::vector<WorldgenInspectionRecord> rows_;
};
}
