// Intended function: Gather immutable committed subsystem snapshots into versioned save publication envelopes with stable section identity.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct SaveSectionIntent {
    std::uint64_t intentId{};
    std::uint64_t sectionId{};
    std::uint64_t schema{};
    std::uint64_t revision{};
    std::uint64_t payloadHash{};
    std::uint64_t flags{};
};
class SaveSectionIntentIndex {
public:
 bool upsert(SaveSectionIntent value); bool erase(std::uint64_t id); [[nodiscard]] const SaveSectionIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<SaveSectionIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const SaveSectionIntent& value) noexcept; std::vector<SaveSectionIntent> rows_;
};
}
