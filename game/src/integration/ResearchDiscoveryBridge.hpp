// Intended function: Translate scans, experiments, specimen analysis, and research completion into discoveries, unlocks, maps, and Chronicle events.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct DiscoveryIntent {
    std::uint64_t intentId{};
    std::uint64_t researchId{};
    std::uint64_t sourceId{};
    std::uint64_t discoveryId{};
    double confidence{};
    std::uint64_t flags{};
};
class DiscoveryIntentIndex {
public:
 bool upsert(DiscoveryIntent value); bool erase(std::uint64_t id); [[nodiscard]] const DiscoveryIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<DiscoveryIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const DiscoveryIntent& value) noexcept; std::vector<DiscoveryIntent> rows_;
};
}
