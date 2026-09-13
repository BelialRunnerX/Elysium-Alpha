// Intended function: Represent versioned stable-entity snapshots independent of transient ECS handles and renderer identities.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::save {
struct EntitySnapshotRecord {
    std::uint64_t stableId{};
    std::uint64_t typeId{};
    std::uint64_t schema{};
    std::uint64_t revision{};
    std::uint64_t componentHash{};
    std::uint64_t payloadBytes{};
};
class EntitySnapshotRecordRegistry {
public:
    bool publish(EntitySnapshotRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const EntitySnapshotRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<EntitySnapshotRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const EntitySnapshotRecord& r) noexcept;
    std::vector<EntitySnapshotRecord> records_;
};
} // namespace elysium::save
