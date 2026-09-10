// Intended function: deterministic ecology far/near representation conversion without deleting named history.
#include "wildlife/EcologyPromotion.hpp"

#include "core/Determinism.hpp"
#include "core/Saturating.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::wildlife {

EcologyPromotionResult EcologyPromotion::promote(
    std::uint64_t habitatSeed,
    PopulationTendency t,
    std::size_t localBudget) const {
    t.expectedPopulation=safe::nonNegative(t.expectedPopulation);
    t.carryingCapacity=safe::nonNegative(t.carryingCapacity);
    t.health01=safe::finiteClamp(t.health01,0.0,1.0);
    const auto wanted=static_cast<std::uint64_t>(std::floor(std::min(t.expectedPopulation,t.carryingCapacity)));
    const auto count=std::min<std::uint64_t>(wanted,static_cast<std::uint64_t>(localBudget));
    EcologyPromotionResult out{}; out.members.reserve(static_cast<std::size_t>(count));
    for(std::uint64_t i=0;i<count;++i){
        auto id=mix64(habitatSeed^mix64(t.speciesKey)^mix64(i+1)); if(id==0)id=1;
        out.members.push_back({id,t.speciesKey,t.health01,false});
    }
    out.residualExpectedPopulation=safe::nonNegative(t.expectedPopulation-static_cast<double>(count));
    return out;
}

PopulationTendency EcologyPromotion::demote(
    PopulationTendency prior,
    const std::vector<LocalPopulationMember>& members,
    double unresolved) const noexcept {
    // Significant individuals are expected to be retained separately by identity/history
    // owners; anonymous members collapse back into the statistical tendency.
    std::uint64_t anonymous=0; double health=0.0;
    for(const auto&m:members) if(!m.historySignificant){++anonymous;health+=safe::finiteClamp(m.health01,0.0,1.0);}
    prior.expectedPopulation=safe::nonNegative(unresolved+static_cast<double>(anonymous));
    prior.health01=anonymous?safe::finiteClamp(health/static_cast<double>(anonymous),0.0,1.0):safe::finiteClamp(prior.health01,0.0,1.0);
    prior.carryingCapacity=safe::nonNegative(prior.carryingCapacity);
    prior.migrationPressure=safe::nonNegative(prior.migrationPressure);
    return prior;
}

} // namespace elysium::wildlife
