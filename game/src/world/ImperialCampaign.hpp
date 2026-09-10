// Intended function: imported world implementation for ImperialCampaign; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/SurfaceSiege.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

// Stable simulation identities. These values are save/network-facing keys and
// must never be populated from entt::entity or other runtime-local handles.
using ImperialSystemId = std::uint64_t;
using ImperialPlanetId = std::uint64_t;
using ImperialStableId = std::uint64_t;

enum class StandingMeter : std::uint8_t {
    Favor = 0,
    Suspicion = 1
};

enum class StandingEventKind : std::uint8_t {
    NamedEmpireKill = 0,
    NamedUnswornKill = 1,
    OrdinaryHostileFavor = 2,
    MineRichOre = 3,
    MineOrdinaryOre = 4,
    SocketGear = 5,
    ReforgeGear = 6,
    AscendGear = 7,
    FileDiscoveryEmpire = 8,
    ClaimEstablished = 9,
    IndustrialActivity = 10,
    CourtTribute = 11,
    ExplicitFavor = 12,
    ExplicitSuspicion = 13
};

struct StandingEvent {
    StandingEventKind kind{StandingEventKind::ExplicitSuspicion};
    ImperialSystemId systemId{};
    // Used by IndustrialActivity and the Explicit* variants. For the fixed
    // inherited events, zero means "use the specification value".
    float amount{};
    // Presence/passive hooks multiply positive gains. Fixed integer-like
    // events are rounded with a floor of one, matching the inherited contract.
    float presenceScale{1.0f};
    float passiveScale{1.0f};
    // Player-placed ore is an explicit no-reward/no-Suspicion anti-exploit
    // path. The caller is responsible for determining authored provenance.
    bool playerPlacedResource{};
    // Court tribute must identify which meter the envoy reads.
    StandingMeter courtMeter{StandingMeter::Favor};
};

struct StandingChange {
    StandingMeter meter{StandingMeter::Suspicion};
    ImperialSystemId systemId{};
    float before{};
    float after{};
    float appliedDelta{};
    bool ignored{};
};

struct SystemSuspicionRecord {
    ImperialSystemId systemId{};
    float value{};

    bool operator==(const SystemSuspicionRecord&) const = default;
};

// Favor is galactic; Suspicion is territorial and keyed by stable system ID.
// Storage is a sorted sparse vector so deterministic iteration never depends on
// hash-table order and an untouched galaxy does not allocate per-system state.
class ImperialStandingLedger {
public:
    float favor() const { return favor_; }
    void setFavor(float value);
    float suspicion(ImperialSystemId systemId) const;
    void setSuspicion(ImperialSystemId systemId, float value);
    float maxSuspicion() const;
    const std::vector<SystemSuspicionRecord>& systems() const { return systems_; }

    StandingChange apply(const StandingEvent& event, float claimFloor = 0.0f);
    bool decaySuspicion(ImperialSystemId systemId, float amount, float claimFloor = 0.0f);

    static ImperialAttentionBand bandFor(float value);
    static float lowestInBand(ImperialAttentionBand band);
    static float dispatchChance(float suspicion);

    std::string serializeState() const;
    bool restoreState(std::string_view text, std::string* error = nullptr);

private:
    float favor_{};
    std::vector<SystemSuspicionRecord> systems_;

    std::vector<SystemSuspicionRecord>::iterator findOrInsert(ImperialSystemId systemId);
};

enum class CourtEnvoyId : std::uint8_t {
    Elysomnion = 0,
    SylpharaVoss = 1,
    Sentinel = 2,
    Lillith = 3,
    Aurelia = 4,
    Count = 5
};

const char* courtEnvoyName(CourtEnvoyId envoy);

enum class ClaimRegisterResult : std::uint8_t {
    Claimed = 0,
    AlreadyClaimed = 1,
    SlotLimit = 2,
    InvalidIdentity = 3
};

struct ImperialClaimRecord {
    ImperialStableId claimId{};
    ImperialSystemId systemId{};
    ImperialPlanetId planetId{};
    ImperialStableId beaconStableId{};
    std::uint64_t structureVoxels{};
    bool active{};

    bool operator==(const ImperialClaimRecord&) const = default;
};

class ImperialClaimRegistry {
public:
    explicit ImperialClaimRegistry(std::uint64_t campaignSeed = 0);

    int slotLimit() const;
    int activeClaimCount() const;
    bool canClaim() const { return activeClaimCount() < slotLimit(); }

    void setEnvoyBefriended(CourtEnvoyId envoy, bool value);
    bool envoyBefriended(CourtEnvoyId envoy) const;

    ClaimRegisterResult registerClaim(ImperialSystemId systemId,
                                      ImperialPlanetId planetId,
                                      ImperialStableId beaconStableId,
                                      std::uint64_t structureVoxels,
                                      ImperialStableId* outClaimId = nullptr);
    bool abandonPlanet(ImperialPlanetId planetId);
    bool strikeBeacon(ImperialStableId beaconStableId);
    bool updateStructureVoxels(ImperialPlanetId planetId, std::uint64_t structureVoxels);

    bool isClaimed(ImperialPlanetId planetId) const;
    const ImperialClaimRecord* claimForPlanet(ImperialPlanetId planetId) const;
    float suspicionFloor(ImperialSystemId systemId) const;
    const std::vector<ImperialClaimRecord>& claims() const { return claims_; }

    std::string serializeState() const;
    bool restoreState(std::string_view text, std::string* error = nullptr);

private:
    std::uint64_t campaignSeed_{};
    std::uint64_t nextClaimSerial_{1};
    std::array<bool, static_cast<std::size_t>(CourtEnvoyId::Count)> befriended_{};
    std::vector<ImperialClaimRecord> claims_;

    ImperialStableId allocateClaimId();
};

enum class ImperialFactKind : std::uint8_t {
    Claim = 0,
    HighVolumeExtraction = 1,
    RegisteredTrade = 2,
    FiledDiscovery = 3,
    Contraband = 4,
    Warrant = 5,
    StationVisit = 6,
    WitnessReport = 7,
    CourtArrangement = 8,
    InspectionResult = 9
};

struct ImperialFactRecord {
    ImperialStableId factId{};
    ImperialSystemId systemId{};
    ImperialFactKind kind{ImperialFactKind::Claim};
    ImperialStableId sourceStableId{};
    float confidence{1.0f};
    bool active{true};

    bool operator==(const ImperialFactRecord&) const = default;
};

// The Imperial file is intentionally not objective truth. Only facts learned
// through explicit sources are stored, allowing future concealment/falsification
// without corrupting the world model itself.
class ImperialFileKnowledge {
public:
    explicit ImperialFileKnowledge(std::uint64_t campaignSeed = 0);

    ImperialStableId learn(ImperialSystemId systemId,
                           ImperialFactKind kind,
                           ImperialStableId sourceStableId = 0,
                           float confidence = 1.0f);
    bool suppress(ImperialStableId factId);
    bool knows(ImperialSystemId systemId, ImperialFactKind kind) const;
    std::size_t activeFactCount(ImperialSystemId systemId) const;
    const std::vector<ImperialFactRecord>& facts() const { return facts_; }

    std::string serializeState() const;
    bool restoreState(std::string_view text, std::string* error = nullptr);

private:
    std::uint64_t campaignSeed_{};
    std::uint64_t nextFactSerial_{1};
    std::vector<ImperialFactRecord> facts_;

    ImperialStableId allocateFactId();
};

enum class CourtReadGate : std::uint8_t {
    Always = 0,
    FavorRecognised = 1,
    FavorExalted = 2,
    SuspicionNoted = 3,
    SuspicionHunted = 4
};

struct CourtEnvoyDefinition {
    CourtEnvoyId id{CourtEnvoyId::Sentinel};
    CourtReadGate gate{CourtReadGate::Always};
    StandingMeter reads{StandingMeter::Suspicion};
};

const CourtEnvoyDefinition& courtEnvoyDefinition(CourtEnvoyId envoy);

enum class CourtOfferKind : std::uint8_t {
    Exchange = 0,
    ConstructionCommission = 1
};

struct CourtOffer {
    ImperialStableId offerId{};
    CourtOfferKind kind{CourtOfferKind::Exchange};
    int rewardTier{};
    int tributeUnits{1};
    std::uint32_t revision{};

    bool operator==(const CourtOffer&) const = default;
};

struct CourtVisit {
    ImperialStableId visitId{};
    CourtEnvoyId envoy{CourtEnvoyId::Sentinel};
    ImperialSystemId hostSystemId{};
    float secondsRemaining{};
    bool handPlaced{};
    CourtOffer offer{};

    bool operator==(const CourtVisit&) const = default;
};

struct CourtTuning {
    float considerationSeconds{300.0f}; // inherited 5-minute cadence
    float considerationChance{0.35f};
    float visitSeconds{1200.0f};        // inherited 20-minute visit
    int maximumRewardTier{3};
};

struct CourtTradeResult {
    bool accepted{};
    StandingChange standing{};
    CourtOffer previousOffer{};
    CourtOffer nextOffer{};
};

// Portable Court scheduler/offer state. It deliberately outputs abstract offer
// descriptors; content/reward registries resolve those descriptors elsewhere.
class CourtService {
public:
    explicit CourtService(std::uint64_t campaignSeed = 0, CourtTuning tuning = {});

    bool eligible(CourtEnvoyId envoy, const ImperialStandingLedger& standing) const;
    std::vector<CourtEnvoyId> eligibleEnvoys(const ImperialStandingLedger& standing) const;

    // Advances an active visit and, on deterministic consideration intervals,
    // may schedule one eligible envoy. No envoy is scheduled merely to refuse.
    std::optional<CourtVisit> update(float dt,
                                     ImperialSystemId hostSystemId,
                                     const ImperialStandingLedger& standing);
    bool summon(CourtEnvoyId envoy,
                ImperialSystemId hostSystemId,
                const ImperialStandingLedger& standing,
                bool handPlaced = false);
    CourtTradeResult acceptCurrentOffer(ImperialStandingLedger& standing);
    void dismiss();

    const std::optional<CourtVisit>& activeVisit() const { return activeVisit_; }
    const CourtTuning& tuning() const { return tuning_; }

    // "Fully befriended" is intentionally an explicit relationship flag. The
    // Fourth Edition does not define a numeric friendship threshold, so this
    // service does not invent one from trade count.
    void setBefriended(CourtEnvoyId envoy, bool value);
    bool befriended(CourtEnvoyId envoy) const;

    std::string serializeState() const;
    bool restoreState(std::string_view text, std::string* error = nullptr);

private:
    std::uint64_t campaignSeed_{};
    std::uint64_t nextVisitSerial_{1};
    std::uint64_t nextOfferSerial_{1};
    std::uint64_t considerationIndex_{};
    float considerationAccumulator_{};
    CourtTuning tuning_{};
    std::optional<CourtVisit> activeVisit_;
    std::array<bool, static_cast<std::size_t>(CourtEnvoyId::Count)> befriended_{};

    ImperialStableId allocateVisitId();
    ImperialStableId allocateOfferId();
    CourtOffer rollOffer(CourtEnvoyId envoy,
                         const ImperialStandingLedger& standing,
                         std::uint32_t revision);
};

enum class CustomsDisposition : std::uint8_t {
    Clear = 0,
    Inspect = 1,
    DetainForWarrant = 2,
    Interdict = 3
};

struct ImperialBureaucracyDecision {
    CustomsDisposition disposition{CustomsDisposition::Clear};
    bool inspectionRecommended{};
    bool routeInterdictionActive{};
    int knownFactCount{};
};

// Read-only policy hooks for future stations, archives, customs and travel.
// They produce decisions; they do not own UI, routes, combat actors or cargo.
ImperialBureaucracyDecision evaluateImperialBureaucracy(
    ImperialSystemId systemId,
    const ImperialStandingLedger& standing,
    const ImperialFileKnowledge& file,
    bool carryingKnownContraband,
    bool activeWarrant);

struct ImperialEnforcementRecord {
    ImperialSystemId systemId{};
    std::string directorState;

    bool operator==(const ImperialEnforcementRecord&) const = default;
};

// One portable persistence envelope for the territorial Empire layer. Active
// SurfaceSiegeDirector state is carried as a versioned stable-ID blob so a game
// save can persist announcement/wave/timer/action/enemy identity without ever
// serializing ECS handles.
class ImperialCampaignState {
public:
    explicit ImperialCampaignState(std::uint64_t campaignSeed = 0,
                                   CourtTuning courtTuning = {});

    ImperialStandingLedger standing;
    ImperialClaimRegistry claims;
    ImperialFileKnowledge fileKnowledge;
    CourtService court;

    void setEnforcementState(ImperialSystemId systemId, std::string directorState);
    const std::string* enforcementState(ImperialSystemId systemId) const;
    bool eraseEnforcementState(ImperialSystemId systemId);
    const std::vector<ImperialEnforcementRecord>& enforcement() const { return enforcement_; }

    void syncClaimSlotsFromCourt();

    std::string serializeState() const;
    bool restoreState(std::string_view text, std::string* error = nullptr);

private:
    std::uint64_t campaignSeed_{};
    std::vector<ImperialEnforcementRecord> enforcement_;
};

} // namespace elysium
