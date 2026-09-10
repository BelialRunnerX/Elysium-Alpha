#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent future deterministic state-delta envelopes keyed by stable IDs and versioned component payloads.
struct SnapshotDeltaCodecOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SnapshotDeltaCodecData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SnapshotDeltaCodecStore { public: bool apply(const SnapshotDeltaCodecOp&); bool erase(std::uint64_t); const SnapshotDeltaCodecData* find(std::uint64_t) const; std::vector<SnapshotDeltaCodecData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SnapshotDeltaCodecData> data_; };
}
