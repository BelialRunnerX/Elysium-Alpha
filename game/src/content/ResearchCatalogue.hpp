// Intended function: Provide research definitions with prerequisites, costs, disciplines, outputs, and Chronicle significance.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::content {
struct ResearchRecord {
    std::uint64_t researchId{};
    std::uint64_t discipline{};
    std::uint64_t prerequisiteHash{};
    std::uint64_t cost{};
    std::uint64_t unlockHash{};
    std::uint64_t flags{};
};
class ResearchRecordRegistry {
public:
    bool publish(ResearchRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const ResearchRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<ResearchRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const ResearchRecord& r) noexcept;
    std::vector<ResearchRecord> records_;
};
} // namespace elysium::content
