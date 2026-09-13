// Intended function: Represent future multiplayer interest regions, stable replicated objects, priority, relevancy, and bandwidth estimates.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::network {
struct ReplicationInterestRecord {
    std::uint64_t observerId{};
    std::uint64_t objectId{};
    std::uint64_t priority{};
    double distance{};
    double bandwidthCost{};
    std::uint64_t flags{};
};
class ReplicationInterestRecordRegistry {
public:
    bool publish(ReplicationInterestRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const ReplicationInterestRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<ReplicationInterestRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const ReplicationInterestRecord& r) noexcept;
    std::vector<ReplicationInterestRecord> records_;
};
} // namespace elysium::network
