// Intended function: Represent future world snapshot sections for chunks, stable objects, entities, strategic summaries, and content fingerprints.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::network {
struct WorldSnapshotSection {
    std::uint64_t sectionId{};
    std::uint64_t snapshotId{};
    std::uint64_t schema{};
    std::uint64_t revision{};
    std::uint64_t payloadHash{};
    std::uint64_t flags{};
};
class WorldSnapshotSectionCollection {
public:
 bool store(WorldSnapshotSection value); bool erase(std::uint64_t id); [[nodiscard]] const WorldSnapshotSection* find(std::uint64_t id) const; [[nodiscard]] const std::vector<WorldSnapshotSection>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const WorldSnapshotSection& v) noexcept; std::vector<WorldSnapshotSection> rows_;
};
}
