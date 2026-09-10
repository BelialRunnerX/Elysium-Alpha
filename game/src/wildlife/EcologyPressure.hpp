// Intended function: Aggregate remote predator/prey, carrying capacity, depletion, invasive pressure, protection, and recolonization.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::wildlife {
struct EcologyPressureState {
    std::uint64_t regionId{};
    std::uint64_t speciesId{};
    double population{};
    double capacity{};
    double predation{};
    double depletion{};
};
class EcologyPressureStateStore {
public:
 bool put(EcologyPressureState v); bool erase(std::uint64_t id);
 [[nodiscard]] const EcologyPressureState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<EcologyPressureState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const EcologyPressureState& v) noexcept; std::vector<EcologyPressureState> values_;
};
} // namespace elysium::wildlife
