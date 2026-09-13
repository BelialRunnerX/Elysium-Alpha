// Intended function: imported survival implementation for Survival; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace elysium {

enum class SurvivalHazard : std::uint8_t {
    Thermal = 0,
    Cryogenic,
    Corrosive,
    Radiological,
    Pressure,
    Vacuum,
    Count
};

constexpr std::size_t kSurvivalHazardCount = static_cast<std::size_t>(SurvivalHazard::Count);
constexpr std::size_t kElementalSurvivalHazardCount = static_cast<std::size_t>(SurvivalHazard::Vacuum);

const char* survivalHazardName(SurvivalHazard hazard);

// Percentage protection sources are independent shares. They are deliberately
// kept separate so gear, modules, food buffs, base engineering and future
// passive hooks can compose without one subsystem knowing another's content.
struct HazardProtectionSources {
    std::array<float,kSurvivalHazardCount> armour{};
    std::array<float,kSurvivalHazardCount> modules{};
    std::array<float,kSurvivalHazardCount> consumables{};
    std::array<float,kSurvivalHazardCount> shelter{};

    // A blocked hazard is an environmental fact, not an additional resistance
    // percentage. Example: an actually sealed/conditioned room may remove an
    // exterior exposure entirely. Agent 10 owns the room/environment query.
    std::array<bool,kSurvivalHazardCount> blocked{};
};

float combineIndependentShares(float a, float b);
float combinedHazardResistance(const HazardProtectionSources& sources, SurvivalHazard hazard);

struct AtmosphereSupport {
    // True only when the shared room/environment service says the local sample
    // satisfies its breathable contract.
    bool breathable{};
    // 0..1 partial support for environments that are not yet fully breathable.
    // It reduces external oxygen drain but never invents pressure/O2 itself.
    float supportFraction{};
};

struct SurvivalEnvironmentInput {
    // Final raw exposure intensities after planet/weather/depth/local-field
    // evaluation. Survival does not duplicate climate, room, fluid or weather
    // simulation. Vacuum is represented by oxygenDrainPerSecond instead of
    // direct elemental damage.
    std::array<float,kSurvivalHazardCount> hazardIntensityPerSecond{};
    float oxygenDrainPerSecond{};
    AtmosphereSupport atmosphere{};
};

struct SurvivalActivityInput {
    bool sprinting{};
    bool moving{};
    float extraEnergyDrainPerSecond{};
    float extraHungerDrainPerSecond{};
    float extraOxygenDrainPerSecond{};
};

struct SurvivalRecoveryInput {
    float oxygenPerSecond{};
    float hazardShieldPerSecond{};
    float energyPerSecond{};
    float hungerPerSecond{};
};

struct SurvivalTickInput {
    SurvivalEnvironmentInput environment{};
    HazardProtectionSources protection{};
    SurvivalActivityInput activity{};
    SurvivalRecoveryInput recovery{};
};

struct SurvivalState {
    float oxygen{100.0f};
    float hazardShielding{100.0f};
    float energy{100.0f};
    float hunger{100.0f};
};

struct SurvivalClock {
    double accumulatorSeconds{};
};

struct SurvivalTuning {
    float fixedStepSeconds{0.1f};
    float maxAcceptedAdvanceSeconds{10.0f};

    float hungerDrainPerSecond{0.12f};
    float coldHungerDrainPerHazardUnit{0.04f};
    float sprintEnergyDrainPerSecond{13.0f};
    float passiveEnergyRecoveryPerSecond{8.0f};
    float breathableOxygenRecoveryPerSecond{7.0f};
    float partialAtmosphereDrainReduction{0.75f};

    float hypoxiaDamagePerSecond{9.0f};
    float starvationDamagePerSecond{1.5f};
    float hazardShieldDrainScale{1.0f};
    float rawHazardHealthDamageScale{1.0f};

    float maxOxygen{100.0f};
    float maxHazardShielding{100.0f};
    float maxEnergy{100.0f};
    float maxHunger{100.0f};
};

enum class SurvivalDamageCause : std::uint8_t {
    Hypoxia = 0,
    Starvation,
    Thermal,
    Cryogenic,
    Corrosive,
    Radiological,
    Pressure,
    Count
};

constexpr std::size_t kSurvivalDamageCauseCount = static_cast<std::size_t>(SurvivalDamageCause::Count);
const char* survivalDamageCauseName(SurvivalDamageCause cause);

enum class SurvivalGauge : std::uint8_t {
    Oxygen = 0,
    HazardShielding,
    Energy,
    Hunger
};

enum class SurvivalEventKind : std::uint8_t {
    GaugeDepleted = 0,
    GaugeRecovered,
    InvalidDeltaTime
};

struct SurvivalEvent {
    SurvivalEventKind kind{SurvivalEventKind::GaugeDepleted};
    SurvivalGauge gauge{SurvivalGauge::Oxygen};
};

// Survival owns exposure arithmetic, not Health/body mutation. Consumers turn
// these explicit requests into combat/medical/body events in their own commit
// phase. The aggregate array in SurvivalTickResult is retained as a cheap
// compatibility/readback surface for the existing direct-operative adapter.
struct SurvivalDamageEvent {
    SurvivalDamageCause cause{SurvivalDamageCause::Hypoxia};
    float amount{};
};

struct HazardDiagnostic {
    float rawRate{};
    float resistance{};
    float mitigatedRate{};
    bool blocked{};
};

struct SurvivalDiagnostics {
    std::array<HazardDiagnostic,kSurvivalHazardCount> hazards{};
    float totalRawElementalHazardRate{};
    float totalMitigatedElementalHazardRate{};
    float oxygenNetDrainPerSecond{}; // positive drains; negative restores
    float energyNetDrainPerSecond{};
    float hungerNetDrainPerSecond{};
    float hazardShieldNetDrainPerSecond{};
    float timeToOxygenFailureSeconds{std::numeric_limits<float>::infinity()};
    float timeToHazardShieldFailureSeconds{std::numeric_limits<float>::infinity()};
    float timeToEnergyFailureSeconds{std::numeric_limits<float>::infinity()};
    float timeToHungerFailureSeconds{std::numeric_limits<float>::infinity()};
    SurvivalHazard dominantHazard{SurvivalHazard::Thermal};
    bool hasElementalHazard{};
};

enum class SurvivalCriticalReserve : std::uint8_t {
    None = 0,
    Oxygen,
    HazardShielding,
    Energy,
    Hunger
};

// Cheap non-mutating projection for remote sites, expedition planning and UI.
// It deliberately predicts reserve exhaustion rather than body damage: once a
// lethal threshold is crossed the caller should promote the actor/site to the
// detailed simulation or apply its own strategic consequence model.
struct SurvivalStrategicProjection {
    SurvivalState projectedState{};
    float horizonSeconds{};
    float timeToFirstCriticalReserveSeconds{std::numeric_limits<float>::infinity()};
    SurvivalCriticalReserve firstCriticalReserve{SurvivalCriticalReserve::None};
    SurvivalHazard dominantHazard{SurvivalHazard::Thermal};
    bool hasElementalHazard{};
    bool requiresDetailedResolution{};
};

SurvivalStrategicProjection projectSurvival(const SurvivalState& state,
                                             const SurvivalTickInput& input,
                                             float horizonSeconds,
                                             const SurvivalTuning& tuning = {});

struct SurvivalTickResult {
    SurvivalDiagnostics diagnostics{};
    std::array<float,kSurvivalDamageCauseCount> damageRequested{};
    std::vector<SurvivalDamageEvent> damageEvents;
    std::vector<SurvivalEvent> events;
    int fixedStepsExecuted{};
    float simulatedSeconds{};
    bool acceptedDeltaTime{true};

    float totalDamageRequested() const;
};

SurvivalDiagnostics inspectSurvival(const SurvivalState& state,
                                    const SurvivalTickInput& input,
                                    const SurvivalTuning& tuning = {});

class SurvivalSystem {
public:
    explicit SurvivalSystem(SurvivalTuning tuning = {});

    const SurvivalTuning& tuning() const { return tuning_; }

    // Active-simulation path. State changes only in fixed-size simulation steps;
    // frame-sized remainder is retained in SurvivalClock. Negative, non-finite,
    // zero-sized or implausibly large deltas are rejected rather than silently
    // changing survival truth.
    SurvivalTickResult advance(SurvivalState& state,
                               SurvivalClock& clock,
                               float dtSeconds,
                               const SurvivalTickInput& input) const;

    // Resource application helpers deliberately do not consume inventory.
    // Inventory/crafting systems own item removal and call these only after a
    // successful transaction.
    float applyOxygenCanister(SurvivalState& state, float amount) const;
    float applyShieldCell(SurvivalState& state, float amount) const;
    float applyCharge(SurvivalState& state, float amount) const;
    float applyFood(SurvivalState& state, float amount) const;

private:
    SurvivalTuning tuning_{};
};

} // namespace elysium
