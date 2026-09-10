// Intended function: Track Favor, Suspicion, warrants, registration state, decay rules, and threshold-triggered Imperial consequences.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::strategy {

struct StandingRecord {
    std::uint64_t systemId{};
    std::uint64_t favor{};
    std::uint64_t suspicion{};
    std::uint64_t warrantLevel{};
    std::uint64_t flags{};
    std::uint64_t lastChangeTick{};
};

class StandingRecordStore {
public:
    bool upsert(StandingRecord value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const StandingRecord* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<StandingRecord> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const StandingRecord& value) noexcept;
    std::vector<StandingRecord> records_;
};

} // namespace elysium::strategy
