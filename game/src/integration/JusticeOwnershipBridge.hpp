// Intended function: Translate stable item ownership/provenance/access facts into theft, contraband, evidence, custody, and restitution intents.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct JusticeEvidenceIntent {
    std::uint64_t intentId{};
    std::uint64_t caseId{};
    std::uint64_t assetId{};
    std::uint64_t ownerId{};
    std::uint64_t suspectId{};
    double confidence{};
};
class JusticeEvidenceIntentIndex {
public:
 bool upsert(JusticeEvidenceIntent value); bool erase(std::uint64_t id); [[nodiscard]] const JusticeEvidenceIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<JusticeEvidenceIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const JusticeEvidenceIntent& value) noexcept; std::vector<JusticeEvidenceIntent> rows_;
};
}
