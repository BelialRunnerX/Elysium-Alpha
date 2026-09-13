// Intended function: Track artifact maker, materials, sites, owners, battles, repairs, inscriptions, and ownership transitions.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::history {
struct ArtifactProvenanceRecord {
    std::uint64_t artifactId{};
    std::uint64_t makerId{};
    std::uint64_t originSiteId{};
    std::uint64_t currentOwnerId{};
    std::uint64_t eventHash{};
    std::uint64_t revision{};
};
class ArtifactProvenanceRecordRegistry {
public:
    bool publish(ArtifactProvenanceRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const ArtifactProvenanceRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<ArtifactProvenanceRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const ArtifactProvenanceRecord& r) noexcept;
    std::vector<ArtifactProvenanceRecord> records_;
};
} // namespace elysium::history
