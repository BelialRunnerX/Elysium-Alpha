// Intended function: Represent future session authority ownership, command sequence windows, reconciliation ticks, and migration state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::network {
struct AuthorityRecord {
    std::uint64_t stableId{};
    std::uint64_t ownerPeerId{};
    std::uint64_t authorityEpoch{};
    std::uint64_t lastCommandSeq{};
    std::uint64_t lastTick{};
    std::uint64_t flags{};
};
class AuthorityRecordRegistry {
public:
    bool publish(AuthorityRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const AuthorityRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<AuthorityRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const AuthorityRecord& r) noexcept;
    std::vector<AuthorityRecord> records_;
};
} // namespace elysium::network
