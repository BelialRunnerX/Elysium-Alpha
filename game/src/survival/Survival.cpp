// Intended function: imported survival implementation for Survival; preserves the agent-authored subsystem contract for later integration/debugging.
#include "survival/Survival.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace elysium {
namespace {

constexpr float kEpsilon = 1.0e-6f;

std::size_t indexOf(SurvivalHazard hazard) {
    return static_cast<std::size_t>(hazard);
}

std::size_t indexOf(SurvivalDamageCause cause) {
    return static_cast<std::size_t>(cause);
}

float finiteNonNegative(float value) {
    return std::isfinite(value) ? std::max(0.0f,value) : 0.0f;
}

float clampedShare(float value) {
    if (!std::isfinite(value)) return 0.0f;
    return std::clamp(value,0.0f,1.0f);
}

float timeToZero(float value, float netDrain) {
    if (netDrain <= kEpsilon) return std::numeric_limits<float>::infinity();
    return std::max(0.0f,value) / netDrain;
}

SurvivalDamageCause damageCauseFor(SurvivalHazard hazard) {
    switch(hazard) {
        case SurvivalHazard::Thermal: return SurvivalDamageCause::Thermal;
        case SurvivalHazard::Cryogenic: return SurvivalDamageCause::Cryogenic;
        case SurvivalHazard::Corrosive: return SurvivalDamageCause::Corrosive;
        case SurvivalHazard::Radiological: return SurvivalDamageCause::Radiological;
        case SurvivalHazard::Pressure: return SurvivalDamageCause::Pressure;
        case SurvivalHazard::Vacuum:
        case SurvivalHazard::Count: break;
    }
    return SurvivalDamageCause::Hypoxia;
}

void appendGaugeTransition(std::vector<SurvivalEvent>& events,
                           SurvivalGauge gauge,
                           float before,
                           float after) {
    if (before > kEpsilon && after <= kEpsilon)
        events.push_back({SurvivalEventKind::GaugeDepleted,gauge});
    else if (before <= kEpsilon && after > kEpsilon)
        events.push_back({SurvivalEventKind::GaugeRecovered,gauge});
}

float applyClampedRecovery(float& value, float amount, float maximum) {
    const float safeAmount=finiteNonNegative(amount);
    const float before=std::clamp(value,0.0f,maximum);
    value=std::clamp(before+safeAmount,0.0f,maximum);
    return value-before;
}

void appendDamage(SurvivalTickResult& result, SurvivalDamageCause cause, float amount) {
    amount=finiteNonNegative(amount);
    if(amount<=kEpsilon) return;
    result.damageRequested[indexOf(cause)]+=amount;
    result.damageEvents.push_back({cause,amount});
}

} // namespace

const char* survivalHazardName(SurvivalHazard hazard) {
    switch(hazard) {
        case SurvivalHazard::Thermal: return "Thermal";
        case SurvivalHazard::Cryogenic: return "Cryogenic";
        case SurvivalHazard::Corrosive: return "Corrosive";
        case SurvivalHazard::Radiological: return "Radiological";
        case SurvivalHazard::Pressure: return "Pressure";
        case SurvivalHazard::Vacuum: return "Vacuum";
        case SurvivalHazard::Count: break;
    }
    return "Unknown";
}

const char* survivalDamageCauseName(SurvivalDamageCause cause) {
    switch(cause) {
        case SurvivalDamageCause::Hypoxia: return "Hypoxia";
        case SurvivalDamageCause::Starvation: return "Starvation";
        case SurvivalDamageCause::Thermal: return "Thermal";
        case SurvivalDamageCause::Cryogenic: return "Cryogenic";
        case SurvivalDamageCause::Corrosive: return "Corrosive";
        case SurvivalDamageCause::Radiological: return "Radiological";
        case SurvivalDamageCause::Pressure: return "Pressure";
        case SurvivalDamageCause::Count: break;
    }
    return "Unknown";
}

float combineIndependentShares(float a, float b) {
    a=clampedShare(a);
    b=clampedShare(b);
    return 1.0f-(1.0f-a)*(1.0f-b);
}

float combinedHazardResistance(const HazardProtectionSources& sources, SurvivalHazard hazard) {
    const auto i=indexOf(hazard);
    if (i>=kSurvivalHazardCount) return 0.0f;
    float resistance=0.0f;
    resistance=combineIndependentShares(resistance,sources.armour[i]);
    resistance=combineIndependentShares(resistance,sources.modules[i]);
    resistance=combineIndependentShares(resistance,sources.consumables[i]);
    resistance=combineIndependentShares(resistance,sources.shelter[i]);
    return resistance;
}

SurvivalDiagnostics inspectSurvival(const SurvivalState& state,
                                    const SurvivalTickInput& input,
                                    const SurvivalTuning& tuning) {
    SurvivalDiagnostics out{};
    float dominantRate=-1.0f;

    for(std::size_t i=0;i<kSurvivalHazardCount;++i) {
        const auto hazard=static_cast<SurvivalHazard>(i);
        auto& d=out.hazards[i];
        d.rawRate=finiteNonNegative(input.environment.hazardIntensityPerSecond[i]);
        d.blocked=input.protection.blocked[i];
        d.resistance=d.blocked ? 1.0f : combinedHazardResistance(input.protection,hazard);
        d.mitigatedRate=d.blocked ? 0.0f : d.rawRate*(1.0f-d.resistance);

        if (hazard!=SurvivalHazard::Vacuum) {
            out.totalRawElementalHazardRate+=d.blocked ? 0.0f : d.rawRate;
            out.totalMitigatedElementalHazardRate+=d.mitigatedRate;
            if(d.mitigatedRate>dominantRate) {
                dominantRate=d.mitigatedRate;
                out.dominantHazard=hazard;
            }
        }
    }
    out.hasElementalHazard=out.totalRawElementalHazardRate>kEpsilon;

    const float atmosphereSupport=clampedShare(input.environment.atmosphere.supportFraction);
    if(input.environment.atmosphere.breathable) {
        out.oxygenNetDrainPerSecond=-(finiteNonNegative(tuning.breathableOxygenRecoveryPerSecond)+
                                      finiteNonNegative(input.recovery.oxygenPerSecond));
    } else {
        const float rawOxygenDrain=finiteNonNegative(input.environment.oxygenDrainPerSecond)+
                                   finiteNonNegative(input.activity.extraOxygenDrainPerSecond);
        const float partialReduction=clampedShare(tuning.partialAtmosphereDrainReduction)*atmosphereSupport;
        out.oxygenNetDrainPerSecond=rawOxygenDrain*(1.0f-partialReduction)-
                                    finiteNonNegative(input.recovery.oxygenPerSecond);
    }

    const float sprintDrain=(input.activity.sprinting && input.activity.moving)
        ? finiteNonNegative(tuning.sprintEnergyDrainPerSecond) : 0.0f;
    const float passiveEnergyRecovery=(!input.activity.sprinting || !input.activity.moving)
        ? finiteNonNegative(tuning.passiveEnergyRecoveryPerSecond) : 0.0f;
    out.energyNetDrainPerSecond=sprintDrain+
                                finiteNonNegative(input.activity.extraEnergyDrainPerSecond)-
                                passiveEnergyRecovery-
                                finiteNonNegative(input.recovery.energyPerSecond);

    const float coldLoad=out.hazards[indexOf(SurvivalHazard::Cryogenic)].mitigatedRate;
    out.hungerNetDrainPerSecond=finiteNonNegative(tuning.hungerDrainPerSecond)+
                                finiteNonNegative(input.activity.extraHungerDrainPerSecond)+
                                coldLoad*finiteNonNegative(tuning.coldHungerDrainPerHazardUnit)-
                                finiteNonNegative(input.recovery.hungerPerSecond);

    out.hazardShieldNetDrainPerSecond=out.totalMitigatedElementalHazardRate*
                                      finiteNonNegative(tuning.hazardShieldDrainScale)-
                                      finiteNonNegative(input.recovery.hazardShieldPerSecond);

    out.timeToOxygenFailureSeconds=timeToZero(state.oxygen,out.oxygenNetDrainPerSecond);
    out.timeToHazardShieldFailureSeconds=timeToZero(state.hazardShielding,out.hazardShieldNetDrainPerSecond);
    out.timeToEnergyFailureSeconds=timeToZero(state.energy,out.energyNetDrainPerSecond);
    out.timeToHungerFailureSeconds=timeToZero(state.hunger,out.hungerNetDrainPerSecond);
    return out;
}

SurvivalStrategicProjection projectSurvival(const SurvivalState& state,
                                             const SurvivalTickInput& input,
                                             float horizonSeconds,
                                             const SurvivalTuning& tuning) {
    SurvivalStrategicProjection out{};
    out.projectedState=state;
    out.horizonSeconds=finiteNonNegative(horizonSeconds);
    const SurvivalDiagnostics d=inspectSurvival(state,input,tuning);
    out.dominantHazard=d.dominantHazard;
    out.hasElementalHazard=d.hasElementalHazard;

    struct Candidate {
        SurvivalCriticalReserve reserve;
        float seconds;
    };
    const std::array<Candidate,4> candidates{{
        {SurvivalCriticalReserve::Oxygen,d.timeToOxygenFailureSeconds},
        {SurvivalCriticalReserve::HazardShielding,d.timeToHazardShieldFailureSeconds},
        {SurvivalCriticalReserve::Energy,d.timeToEnergyFailureSeconds},
        {SurvivalCriticalReserve::Hunger,d.timeToHungerFailureSeconds}
    }};
    for(const auto& candidate:candidates) {
        if(candidate.seconds<out.timeToFirstCriticalReserveSeconds) {
            out.timeToFirstCriticalReserveSeconds=candidate.seconds;
            out.firstCriticalReserve=candidate.reserve;
        }
    }

    const float horizon=out.horizonSeconds;
    auto projectGauge=[horizon](float value, float netDrain, float maximum) {
        if(!std::isfinite(value)) value=0.0f;
        const float projected=value-netDrain*horizon;
        return std::clamp(projected,0.0f,finiteNonNegative(maximum));
    };
    out.projectedState.oxygen=projectGauge(state.oxygen,d.oxygenNetDrainPerSecond,tuning.maxOxygen);
    out.projectedState.hazardShielding=projectGauge(state.hazardShielding,d.hazardShieldNetDrainPerSecond,
                                                     tuning.maxHazardShielding);
    out.projectedState.energy=projectGauge(state.energy,d.energyNetDrainPerSecond,tuning.maxEnergy);
    out.projectedState.hunger=projectGauge(state.hunger,d.hungerNetDrainPerSecond,tuning.maxHunger);

    // A strategic projection is only safe while every reserve remains above its
    // threshold. Crossing oxygen/hunger or elemental shielding requires a body/
    // medical consequence model; energy depletion may alter travel/work plans.
    out.requiresDetailedResolution=std::isfinite(out.timeToFirstCriticalReserveSeconds) &&
                                   out.timeToFirstCriticalReserveSeconds<=horizon+kEpsilon;
    return out;
}

float SurvivalTickResult::totalDamageRequested() const {
    float total=0.0f;
    for(float value:damageRequested) total+=value;
    return total;
}

SurvivalSystem::SurvivalSystem(SurvivalTuning tuning) : tuning_(tuning) {
    if(!std::isfinite(tuning_.fixedStepSeconds) || tuning_.fixedStepSeconds<=0.0f)
        throw std::invalid_argument("SurvivalSystem fixedStepSeconds must be finite and > 0");
    if(!std::isfinite(tuning_.maxAcceptedAdvanceSeconds) || tuning_.maxAcceptedAdvanceSeconds<tuning_.fixedStepSeconds)
        throw std::invalid_argument("SurvivalSystem maxAcceptedAdvanceSeconds must be >= fixedStepSeconds");
}

SurvivalTickResult SurvivalSystem::advance(SurvivalState& state,
                                           SurvivalClock& clock,
                                           float dtSeconds,
                                           const SurvivalTickInput& input) const {
    SurvivalTickResult out{};
    out.diagnostics=inspectSurvival(state,input,tuning_);
    if(!std::isfinite(dtSeconds) || dtSeconds<0.0f || dtSeconds>tuning_.maxAcceptedAdvanceSeconds) {
        out.acceptedDeltaTime=false;
        out.events.push_back({SurvivalEventKind::InvalidDeltaTime,SurvivalGauge::Oxygen});
        return out;
    }
    if(dtSeconds==0.0f) return out;

    state.oxygen=std::clamp(state.oxygen,0.0f,tuning_.maxOxygen);
    state.hazardShielding=std::clamp(state.hazardShielding,0.0f,tuning_.maxHazardShielding);
    state.energy=std::clamp(state.energy,0.0f,tuning_.maxEnergy);
    state.hunger=std::clamp(state.hunger,0.0f,tuning_.maxHunger);

    clock.accumulatorSeconds+=static_cast<double>(dtSeconds);
    const double step=static_cast<double>(tuning_.fixedStepSeconds);
    while(clock.accumulatorSeconds+1.0e-9>=step) {
        const SurvivalDiagnostics d=inspectSurvival(state,input,tuning_);
        const float dt=tuning_.fixedStepSeconds;

        const float oxygenBefore=state.oxygen;
        const float shieldBefore=state.hazardShielding;
        const float energyBefore=state.energy;
        const float hungerBefore=state.hunger;

        // Oxygen: if the reserve crosses zero in this step, only the remainder
        // of the step requests hypoxia damage. This keeps the threshold result
        // independent of the render-frame cadence feeding the fixed clock.
        if(d.oxygenNetDrainPerSecond>kEpsilon) {
            const float drain=d.oxygenNetDrainPerSecond;
            const float timeUntilEmpty=state.oxygen/drain;
            if(timeUntilEmpty>=dt) state.oxygen-=drain*dt;
            else {
                state.oxygen=0.0f;
                const float exposed=std::max(0.0f,dt-timeUntilEmpty);
                appendDamage(out,SurvivalDamageCause::Hypoxia,
                             exposed*finiteNonNegative(tuning_.hypoxiaDamagePerSecond));
            }
        } else if(d.oxygenNetDrainPerSecond< -kEpsilon) {
            state.oxygen=std::min(tuning_.maxOxygen,state.oxygen-d.oxygenNetDrainPerSecond*dt);
        }

        // Hunger follows the same threshold split. Energy has no lethal failure
        // state and is simply clamped.
        if(d.hungerNetDrainPerSecond>kEpsilon) {
            const float drain=d.hungerNetDrainPerSecond;
            const float timeUntilEmpty=state.hunger/drain;
            if(timeUntilEmpty>=dt) state.hunger-=drain*dt;
            else {
                state.hunger=0.0f;
                const float starved=std::max(0.0f,dt-timeUntilEmpty);
                appendDamage(out,SurvivalDamageCause::Starvation,
                             starved*finiteNonNegative(tuning_.starvationDamagePerSecond));
            }
        } else if(d.hungerNetDrainPerSecond< -kEpsilon) {
            state.hunger=std::min(tuning_.maxHunger,state.hunger-d.hungerNetDrainPerSecond*dt);
        }

        if(d.energyNetDrainPerSecond>kEpsilon)
            state.energy=std::max(0.0f,state.energy-d.energyNetDrainPerSecond*dt);
        else if(d.energyNetDrainPerSecond< -kEpsilon)
            state.energy=std::min(tuning_.maxEnergy,state.energy-d.energyNetDrainPerSecond*dt);

        // Hazard shielding is the expedition reserve. Mitigated exposure drains
        // it. Once exhausted, remaining elemental exposure in the step requests
        // health damage at the raw (unmitigated) hazard rate, per the inherited
        // survival contract. Direct health ownership remains outside this system.
        if(d.hazardShieldNetDrainPerSecond>kEpsilon) {
            const float drain=d.hazardShieldNetDrainPerSecond;
            const float timeUntilEmpty=state.hazardShielding/drain;
            float rawExposureSeconds=0.0f;
            if(timeUntilEmpty>=dt) state.hazardShielding-=drain*dt;
            else {
                state.hazardShielding=0.0f;
                rawExposureSeconds=std::max(0.0f,dt-timeUntilEmpty);
            }
            if(rawExposureSeconds>0.0f) {
                for(std::size_t i=0;i<kElementalSurvivalHazardCount;++i) {
                    const auto hazard=static_cast<SurvivalHazard>(i);
                    const auto& hd=d.hazards[i];
                    if(hd.blocked || hd.rawRate<=0.0f) continue;
                    const float damage=hd.rawRate*rawExposureSeconds*
                                       finiteNonNegative(tuning_.rawHazardHealthDamageScale);
                    appendDamage(out,damageCauseFor(hazard),damage);
                }
            }
        } else if(d.hazardShieldNetDrainPerSecond< -kEpsilon) {
            state.hazardShielding=std::min(tuning_.maxHazardShielding,
                                           state.hazardShielding-d.hazardShieldNetDrainPerSecond*dt);
        }

        state.oxygen=std::clamp(state.oxygen,0.0f,tuning_.maxOxygen);
        state.hazardShielding=std::clamp(state.hazardShielding,0.0f,tuning_.maxHazardShielding);
        state.energy=std::clamp(state.energy,0.0f,tuning_.maxEnergy);
        state.hunger=std::clamp(state.hunger,0.0f,tuning_.maxHunger);

        appendGaugeTransition(out.events,SurvivalGauge::Oxygen,oxygenBefore,state.oxygen);
        appendGaugeTransition(out.events,SurvivalGauge::HazardShielding,shieldBefore,state.hazardShielding);
        appendGaugeTransition(out.events,SurvivalGauge::Energy,energyBefore,state.energy);
        appendGaugeTransition(out.events,SurvivalGauge::Hunger,hungerBefore,state.hunger);

        clock.accumulatorSeconds-=step;
        if(clock.accumulatorSeconds<0.0 && clock.accumulatorSeconds>-1.0e-8) clock.accumulatorSeconds=0.0;
        ++out.fixedStepsExecuted;
        out.simulatedSeconds+=dt;
    }

    out.diagnostics=inspectSurvival(state,input,tuning_);
    return out;
}

float SurvivalSystem::applyOxygenCanister(SurvivalState& state, float amount) const {
    return applyClampedRecovery(state.oxygen,amount,tuning_.maxOxygen);
}

float SurvivalSystem::applyShieldCell(SurvivalState& state, float amount) const {
    return applyClampedRecovery(state.hazardShielding,amount,tuning_.maxHazardShielding);
}

float SurvivalSystem::applyCharge(SurvivalState& state, float amount) const {
    return applyClampedRecovery(state.energy,amount,tuning_.maxEnergy);
}

float SurvivalSystem::applyFood(SurvivalState& state, float amount) const {
    return applyClampedRecovery(state.hunger,amount,tuning_.maxHunger);
}

} // namespace elysium
