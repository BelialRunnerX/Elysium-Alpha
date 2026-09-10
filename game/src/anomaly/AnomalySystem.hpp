// Intended function: Represent anomalies with discovery state, sensor signatures, effects, research value, hazard, and resolution outcomes.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::anomaly {
struct AnomalyState {
    std::uint64_t anomalyId{};
    std::uint64_t typeId{};
    std::uint64_t signature{};
    double researchValue{};
    double hazard{};
    std::uint64_t state{};
};
class AnomalyStateStore {
public:
 bool put(AnomalyState v); bool erase(std::uint64_t id);
 [[nodiscard]] const AnomalyState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<AnomalyState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const AnomalyState& v) noexcept; std::vector<AnomalyState> values_;
};
} // namespace elysium::anomaly
