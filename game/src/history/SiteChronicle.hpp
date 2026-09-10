// Intended function: Maintain compact site-local historical event streams linked to Galactic Chronicle stable identities.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::history {
struct SiteChronicleRecord {
    std::uint64_t eventId{};
    std::uint64_t siteId{};
    std::uint64_t actorId{};
    std::uint64_t subjectId{};
    std::uint64_t eventType{};
    std::uint64_t tick{};
};
class SiteChronicleRecordRegistry {
public:
    bool publish(SiteChronicleRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const SiteChronicleRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<SiteChronicleRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const SiteChronicleRecord& r) noexcept;
    std::vector<SiteChronicleRecord> records_;
};
} // namespace elysium::history
