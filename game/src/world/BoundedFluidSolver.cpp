// Intended function: deterministic bounded local fluid transfer calculation.
#include "world/BoundedFluidSolver.hpp"

#include <algorithm>
#include <map>
#include <tuple>

namespace elysium {
namespace {
struct AddressLess {
    bool operator()(const LargeSurfaceCellAddress& a, const LargeSurfaceCellAddress& b) const noexcept {
        return std::tie(a.planetId,a.face,a.u,a.v,a.radial) < std::tie(b.planetId,b.face,b.u,b.v,b.radial);
    }
};
}

FluidStepResult BoundedFluidSolver::compute(
    std::span<const FluidCell> cells,
    const IFluidNeighborhood& neighborhood,
    std::size_t transferBudget) const {
    FluidStepResult out{};
    if (transferBudget == 0 || cells.empty()) return out;

    std::map<LargeSurfaceCellAddress, FluidCell, AddressLess> ordered;
    for (auto c : cells) {
        c.volume01 = safe::finiteClamp(c.volume01, 0.0, 1.0);
        c.temperatureK = safe::nonNegative(c.temperatureK);
        c.contamination01 = safe::finiteClamp(c.contamination01, 0.0, 1.0);
        if (c.volume01 > 0.0) ordered[c.address] = c;
    }

    std::size_t issued = 0;
    for (const auto& [address, cell] : ordered) {
        if (issued >= transferBudget) break;
        auto neighbors = neighborhood.neighbors(address);
        std::sort(neighbors.begin(), neighbors.end(), AddressLess{});
        double remaining = cell.volume01;
        for (const auto& next : neighbors) {
            if (issued >= transferBudget || remaining <= 0.0) break;
            if (neighborhood.isSolid(next)) continue;
            const double capacity = safe::finiteClamp(neighborhood.fluidCapacity01(next), 0.0, 1.0);
            if (capacity <= 0.0) continue;
            const bool downward = next.radial < address.radial;
            const double directional = downward ? 0.75 : 0.22;
            const double amount = safe::finiteClamp(std::min(remaining, capacity * directional), 0.0, 1.0);
            if (amount <= 0.0) continue;
            out.transfers.push_back({address, next, cell.kind, amount});
            remaining = safe::finiteClamp(remaining - amount, 0.0, 1.0);
            ++issued;
        }
    }
    return out;
}

} // namespace elysium
