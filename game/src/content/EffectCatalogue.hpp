// Intended function: Provide stable gameplay-effect definitions for buffs, debuffs, hazards, medical states, abilities, and equipment passives.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::content {
struct EffectRecord {
    std::uint64_t effectId{};
    std::uint64_t category{};
    std::uint64_t durationTicks{};
    std::uint64_t stackPolicy{};
    double magnitude{};
    std::uint64_t flags{};
};
class EffectRecordRegistry {
public:
    bool publish(EffectRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const EffectRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<EffectRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const EffectRecord& r) noexcept;
    std::vector<EffectRecord> records_;
};
} // namespace elysium::content
