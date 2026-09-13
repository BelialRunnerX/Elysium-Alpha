// Intended function: Generate deterministic dungeon topology, room graph, locks, hazards, encounter tiers, treasure, and exit guarantees.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::dungeon {
struct DungeonDescriptor {
    std::uint64_t dungeonId{};
    std::uint64_t roomCount{};
    std::uint64_t lockCount{};
    double hazardCount{};
    std::uint64_t encounterTier{};
    std::uint64_t seed{};
};
class DungeonDescriptorStore {
public:
 bool put(DungeonDescriptor v); bool erase(std::uint64_t id);
 [[nodiscard]] const DungeonDescriptor* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<DungeonDescriptor>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const DungeonDescriptor& v) noexcept; std::vector<DungeonDescriptor> values_;
};
} // namespace elysium::dungeon
