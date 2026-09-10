// Intended function: Score bounded action candidates using needs, danger, orders, opportunity, skill, relationships, and deterministic tie-breaks.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ai {
struct UtilityCandidate {
    std::uint64_t candidateId{};
    std::uint64_t actorId{};
    std::uint64_t actionId{};
    double score{};
    double cost{};
    std::uint64_t priority{};
};
class UtilityCandidateTable {
public:
 bool set(UtilityCandidate value); bool remove(std::uint64_t id);
 [[nodiscard]] const UtilityCandidate* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<UtilityCandidate> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const UtilityCandidate& value) noexcept; std::vector<UtilityCandidate> rows_;
};
}
