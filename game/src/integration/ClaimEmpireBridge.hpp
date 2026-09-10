// Intended function: Translate claim/beacon actions, expansion, extraction, violations, and destruction into system-local Suspicion/Favor/Empire consequences.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct ClaimConsequenceIntent {
    std::uint64_t intentId{};
    std::uint64_t claimId{};
    std::uint64_t systemId{};
    std::uint64_t actionType{};
    double suspicionDelta{};
    double favorDelta{};
};
class ClaimConsequenceIntentIndex {
public:
 bool upsert(ClaimConsequenceIntent value); bool erase(std::uint64_t id); [[nodiscard]] const ClaimConsequenceIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ClaimConsequenceIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const ClaimConsequenceIntent& value) noexcept; std::vector<ClaimConsequenceIntent> rows_;
};
}
