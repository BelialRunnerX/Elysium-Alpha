#include "fortress/Systems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

void updateMarketGood(MarketGood& good, float imports, float exports, float localProduction,
                      float localConsumption) {
    good.supply = std::max(0.0f, good.supply + std::max(0.0f, imports) + std::max(0.0f, localProduction) - std::max(0.0f, exports));
    good.demand = std::max(0.01f, good.demand + std::max(0.0f, localConsumption) - std::max(0.0f, imports) * 0.15f);
    const float scarcity = good.demand / std::max(0.1f, good.supply);
    good.price = std::max(0.01f, good.basePrice * std::clamp(std::sqrt(scarcity), 0.25f, 4.0f));
}

ContractState generateContract(const ContractGenerationContext& context, ContractType type,
                               ContractScope scope) {
    ContractState contract{};
    const std::uint64_t discriminator = static_cast<std::uint64_t>(type) * 31ULL + static_cast<std::uint64_t>(scope);
    contract.id = makeDerivedId<ContractId>(context.seed, context.issuer.value, 0x434F4E5452414354ULL ^ discriminator, context.epoch);
    contract.type = type;
    contract.scope = scope;
    contract.issuer = context.issuer;
    contract.targetSite = context.target;
    const float scopeScale = scope == ContractScope::Local ? 1.0f : scope == ContractScope::System ? 2.4f : 5.0f;
    const float risk = 1.0f + saturate(context.hazard) * 1.3f + boundedPercent(context.suspicion) / 100.0f;
    contract.credits = (80.0f + std::max(0.0f, context.wealth) * 0.04f) * scopeScale * risk;
    contract.standingReward = 1.0f * scopeScale * risk;
    contract.objective = ContentId{"elysium:contract/objective_" + std::to_string(static_cast<unsigned>(type))};
    contract.complication = ContentId{context.hazard > 0.6f ? "elysium:contract/complication_hazard" :
                                     context.suspicion > 60.0f ? "elysium:contract/complication_empire" :
                                     "elysium:contract/complication_logistics"};
    return contract;
}

float caravanLossRisk(const CaravanState& caravan, float routeHazard, float hostilePressure) {
    const float exposure = saturate(routeHazard) * 0.45f + saturate(hostilePressure) * 0.45f + saturate(caravan.risk) * 0.25f;
    const float protection = saturate(caravan.escortStrength / std::max(1.0f, caravan.capacity * 0.02f));
    return saturate(exposure * (1.0f - protection * 0.7f));
}

void advanceDiplomacy(DiplomacyState& state, float trade, float borderConflict,
                      float sharedThreat, float ideologicalDistance) {
    state.trust = std::clamp(state.trust + std::max(0.0f, trade) * 0.01f + std::max(0.0f, sharedThreat) * 0.008f -
                             std::max(0.0f, borderConflict) * 0.025f - std::max(0.0f, ideologicalDistance) * 0.004f, -1.0f, 1.0f);
    state.fear = std::clamp(state.fear + std::max(0.0f, sharedThreat) * 0.01f + std::max(0.0f, borderConflict) * 0.008f - std::max(0.0f, trade) * 0.002f, 0.0f, 1.0f);
    state.tradeAffinity = std::clamp(state.tradeAffinity + std::max(0.0f, trade) * 0.012f - state.embargo * 0.02f, -1.0f, 1.0f);
    state.grievance = std::clamp(state.grievance + std::max(0.0f, borderConflict) * 0.02f - std::max(0.0f, trade) * 0.003f, 0.0f, 1.0f);
    if (state.grievance > 0.82f && state.trust < -0.45f) state.atWar = true;
    if (state.atWar && state.grievance < 0.35f && state.trust > 0.1f) state.atWar = false;
}

} // namespace elysium::fortress
