// Intended function: Translate committed simulation outcomes into Chronicle-worthy stable event envelopes without making history authoritative over gameplay.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct ChroniclePublishIntent {
    std::uint64_t intentId{};
    std::uint64_t eventType{};
    std::uint64_t sourceId{};
    std::uint64_t subjectId{};
    std::uint64_t siteId{};
    double importance{};
};
class ChroniclePublishIntentIndex {
public:
 bool upsert(ChroniclePublishIntent value); bool erase(std::uint64_t id); [[nodiscard]] const ChroniclePublishIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ChroniclePublishIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const ChroniclePublishIntent& value) noexcept; std::vector<ChroniclePublishIntent> rows_;
};
}
