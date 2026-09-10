// Intended function: Track archive sections, schema versions, generator fingerprints, migration requirements, and integrity digests.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::save {
struct ArchiveSection {
    std::uint64_t sectionId{};
    std::uint64_t schema{};
    std::uint64_t generatorVersion{};
    std::uint64_t fingerprint{};
    std::uint64_t digest{};
    std::uint64_t flags{};
};
class ArchiveSectionRegistry {
public:
    bool publish(ArchiveSection record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const ArchiveSection* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<ArchiveSection>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const ArchiveSection& r) noexcept;
    std::vector<ArchiveSection> records_;
};
} // namespace elysium::save
