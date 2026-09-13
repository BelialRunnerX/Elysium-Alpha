// Intended function: Encode versioned sparse chunk edits, tombstones, object deltas, checksums, and compatibility metadata.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::save {
struct ChunkDeltaRecord {
    std::uint64_t recordId{};
    std::uint64_t chunkKey{};
    std::uint64_t schema{};
    std::uint64_t generation{};
    std::uint64_t checksum{};
    std::uint64_t payloadBytes{};
};
class ChunkDeltaRecordRegistry {
public:
    bool publish(ChunkDeltaRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const ChunkDeltaRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<ChunkDeltaRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const ChunkDeltaRecord& r) noexcept;
    std::vector<ChunkDeltaRecord> records_;
};
} // namespace elysium::save
