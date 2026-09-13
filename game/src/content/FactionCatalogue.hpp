// Intended function: Provide stable faction definitions, ideology tags, default relations, law policies, visuals, and economic archetypes.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::content {
struct FactionRecord {
    std::uint64_t factionId{};
    std::uint64_t ideologyHash{};
    std::uint64_t lawPolicyId{};
    std::uint64_t economyArchetype{};
    std::uint64_t visualThemeId{};
    std::uint64_t flags{};
};
class FactionRecordRegistry {
public:
    bool publish(FactionRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const FactionRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<FactionRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const FactionRecord& r) noexcept;
    std::vector<FactionRecord> records_;
};
} // namespace elysium::content
