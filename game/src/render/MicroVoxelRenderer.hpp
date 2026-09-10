// Intended function: Represent refined 16^3 microvoxel draw packets, surface masks, damage layers, and distance-based simplification.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::render {
struct MicroVoxelDrawRecord {
    std::uint64_t recordId{};
    std::uint64_t chunkKey{};
    std::uint64_t macroCellKey{};
    std::uint64_t surfaceMask{};
    std::uint64_t lodTier{};
    std::uint64_t materialHash{};
};
class MicroVoxelDrawRecordRegistry {
public:
    bool publish(MicroVoxelDrawRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const MicroVoxelDrawRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<MicroVoxelDrawRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const MicroVoxelDrawRecord& r) noexcept;
    std::vector<MicroVoxelDrawRecord> records_;
};
} // namespace elysium::render
