// Intended function: Publish immutable versioned snapshots after Commit for asynchronous persistence without exposing live mutable state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::simulation {
struct SnapshotEnvelope {
    std::uint64_t snapshotId{};
    std::uint64_t tick{};
    std::uint64_t worldRevision{};
    std::uint64_t schema{};
    std::uint64_t payloadHash{};
    std::uint64_t flags{};
};
class SnapshotEnvelopeTable {
public:
 bool set(SnapshotEnvelope value); bool remove(std::uint64_t id);
 [[nodiscard]] const SnapshotEnvelope* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<SnapshotEnvelope> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const SnapshotEnvelope& value) noexcept; std::vector<SnapshotEnvelope> rows_;
};
}
