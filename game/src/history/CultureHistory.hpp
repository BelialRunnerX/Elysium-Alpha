// Intended function: Track cultural works, traditions, institutions, migrations, language shifts, and historical influence.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::history {
struct CultureHistoryRecord {
    std::uint64_t cultureId{};
    std::uint64_t eventId{};
    std::uint64_t siteId{};
    double importance{};
    double influence{};
    std::uint64_t tick{};
};
class CultureHistoryRecordRegistry {
public:
    bool publish(CultureHistoryRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const CultureHistoryRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<CultureHistoryRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const CultureHistoryRecord& r) noexcept;
    std::vector<CultureHistoryRecord> records_;
};
} // namespace elysium::history
