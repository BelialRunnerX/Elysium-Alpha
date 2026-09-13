// Intended function: Represent Court envoys, favor thresholds, commissions, audiences, judgments, ceremonial obligations, and political opportunities.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::faction {
struct CourtState {
    std::uint64_t recordId{};
    std::uint64_t actorId{};
    double favor{};
    std::uint64_t rank{};
    std::uint64_t commissionId{};
    std::uint64_t flags{};
};
class CourtStateTable {
public:
 bool set(CourtState value); bool remove(std::uint64_t id);
 [[nodiscard]] const CourtState* find(std::uint64_t id) const;
 [[nodiscard]] std::vector<CourtState> snapshot() const { return rows_; }
private:
 static std::uint64_t keyOf(const CourtState& value) noexcept; std::vector<CourtState> rows_;
};
}
