// Intended function: Represent immutable component snapshot envelopes for persistence, replication, inspection, and shard transfer.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ecs {
struct ComponentSnapshot {
    std::uint64_t snapshotId{};
    std::uint64_t stableId{};
    std::uint64_t componentType{};
    std::uint64_t schema{};
    std::uint64_t payloadHash{};
    std::uint64_t revision{};
};
class ComponentSnapshotCollection {
public:
 bool store(ComponentSnapshot value); bool erase(std::uint64_t id); [[nodiscard]] const ComponentSnapshot* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ComponentSnapshot>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const ComponentSnapshot& v) noexcept; std::vector<ComponentSnapshot> rows_;
};
}
