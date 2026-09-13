// Intended function: imported world implementation for ImperialCampaign; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/ImperialCampaign.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <utility>

namespace elysium {
namespace {

constexpr std::uint64_t kClaimLabel = 0x434C41494DULL;      // CLAIM
constexpr std::uint64_t kFactLabel = 0x46494C4546414354ULL;  // FILEFACT
constexpr std::uint64_t kVisitLabel = 0x434F555254564953ULL; // COURTVIS
constexpr std::uint64_t kOfferLabel = 0x434F5552544F4646ULL; // COURTOFF

float clampStanding(float value) {
    if (!std::isfinite(value)) return 0.0f;
    return std::clamp(value, 0.0f, 100.0f);
}

float positiveScale(float a, float b) {
    if (!std::isfinite(a) || !std::isfinite(b)) return 1.0f;
    return std::max(0.0f, a) * std::max(0.0f, b);
}

float scaledDiscrete(float base, float scale) {
    if (base <= 0.0f || scale <= 0.0f) return 0.0f;
    return static_cast<float>(std::max(1L, std::lround(base * scale)));
}

std::string hexEncode(std::string_view input) {
    static constexpr char digits[] = "0123456789ABCDEF";
    std::string out;
    out.reserve(input.size() * 2);
    for (const unsigned char c : input) {
        out.push_back(digits[(c >> 4U) & 0x0FU]);
        out.push_back(digits[c & 0x0FU]);
    }
    return out;
}

int hexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

bool hexDecode(std::string_view input, std::string& output) {
    if ((input.size() & 1U) != 0U) return false;
    output.clear();
    output.reserve(input.size() / 2U);
    for (std::size_t i = 0; i < input.size(); i += 2U) {
        const int hi = hexDigit(input[i]);
        const int lo = hexDigit(input[i + 1U]);
        if (hi < 0 || lo < 0) return false;
        output.push_back(static_cast<char>((hi << 4) | lo));
    }
    return true;
}

template <typename T>
bool validEnumInt(int value, T countExclusive) {
    return value >= 0 && value < static_cast<int>(countExclusive);
}

std::size_t envoyIndex(CourtEnvoyId envoy) {
    return static_cast<std::size_t>(envoy);
}

} // namespace

void ImperialStandingLedger::setFavor(float value) {
    favor_ = clampStanding(value);
}

std::vector<SystemSuspicionRecord>::iterator ImperialStandingLedger::findOrInsert(ImperialSystemId systemId) {
    auto it = std::lower_bound(systems_.begin(), systems_.end(), systemId,
        [](const SystemSuspicionRecord& record, ImperialSystemId id) { return record.systemId < id; });
    if (it == systems_.end() || it->systemId != systemId)
        it = systems_.insert(it, SystemSuspicionRecord{systemId, 0.0f});
    return it;
}

float ImperialStandingLedger::suspicion(ImperialSystemId systemId) const {
    const auto it = std::lower_bound(systems_.begin(), systems_.end(), systemId,
        [](const SystemSuspicionRecord& record, ImperialSystemId id) { return record.systemId < id; });
    return it != systems_.end() && it->systemId == systemId ? it->value : 0.0f;
}

void ImperialStandingLedger::setSuspicion(ImperialSystemId systemId, float value) {
    if (systemId == 0) return;
    const float clamped = clampStanding(value);
    auto it = findOrInsert(systemId);
    it->value = clamped;
    // Sparse representation: clear, untouched systems need no record.
    if (clamped == 0.0f) systems_.erase(it);
}

float ImperialStandingLedger::maxSuspicion() const {
    float result = 0.0f;
    for (const auto& record : systems_) result = std::max(result, record.value);
    return result;
}

ImperialAttentionBand ImperialStandingLedger::bandFor(float value) {
    value = clampStanding(value);
    if (value >= 75.0f) return ImperialAttentionBand::Hunted;
    if (value >= 50.0f) return ImperialAttentionBand::Marked;
    if (value >= 25.0f) return ImperialAttentionBand::Noted;
    return ImperialAttentionBand::Quiet;
}

float ImperialStandingLedger::lowestInBand(ImperialAttentionBand band) {
    switch (band) {
        case ImperialAttentionBand::Hunted: return 75.0f;
        case ImperialAttentionBand::Marked: return 50.0f;
        case ImperialAttentionBand::Noted: return 25.0f;
        case ImperialAttentionBand::Quiet: return 0.0f;
    }
    return 0.0f;
}

float ImperialStandingLedger::dispatchChance(float value) {
    value = clampStanding(value);
    return value < 25.0f ? 0.0f : std::min(0.40f, (value - 25.0f) / 200.0f);
}

StandingChange ImperialStandingLedger::apply(const StandingEvent& event, float claimFloor) {
    StandingChange result{};
    result.systemId = event.systemId;
    const float scale = positiveScale(event.presenceScale, event.passiveScale);

    StandingMeter meter = StandingMeter::Suspicion;
    float base = event.amount;
    bool discrete = true;

    switch (event.kind) {
        case StandingEventKind::NamedEmpireKill: meter = StandingMeter::Suspicion; base = base == 0.0f ? 12.0f : base; break;
        case StandingEventKind::NamedUnswornKill: meter = StandingMeter::Favor; base = base == 0.0f ? 4.0f : base; break;
        case StandingEventKind::OrdinaryHostileFavor: meter = StandingMeter::Favor; base = base == 0.0f ? 1.0f : base; break;
        case StandingEventKind::MineRichOre:
            meter = StandingMeter::Suspicion; base = base == 0.0f ? 4.0f : base;
            if (event.playerPlacedResource) result.ignored = true;
            break;
        case StandingEventKind::MineOrdinaryOre:
            meter = StandingMeter::Suspicion; base = base == 0.0f ? 2.0f : base;
            if (event.playerPlacedResource) result.ignored = true;
            break;
        case StandingEventKind::SocketGear: meter = StandingMeter::Favor; base = base == 0.0f ? 2.0f : base; break;
        case StandingEventKind::ReforgeGear: meter = StandingMeter::Favor; base = base == 0.0f ? 2.0f : base; break;
        case StandingEventKind::AscendGear: meter = StandingMeter::Suspicion; base = base == 0.0f ? 8.0f : base; break;
        case StandingEventKind::FileDiscoveryEmpire: meter = StandingMeter::Suspicion; base = base == 0.0f ? 2.0f : base; break;
        case StandingEventKind::ClaimEstablished:
            meter = StandingMeter::Suspicion;
            // A claim's inherited attention cost is its persistent structural
            // floor, not an independent one-off gain. Applying this event
            // therefore raises the territorial meter to the supplied floor.
            base = 0.0f;
            discrete = false;
            break;
        case StandingEventKind::IndustrialActivity:
            meter = StandingMeter::Suspicion;
            discrete = false;
            break;
        case StandingEventKind::CourtTribute:
            meter = event.courtMeter;
            base = base == 0.0f ? 3.0f : base;
            break;
        case StandingEventKind::ExplicitFavor:
            meter = StandingMeter::Favor;
            discrete = false;
            break;
        case StandingEventKind::ExplicitSuspicion:
            meter = StandingMeter::Suspicion;
            discrete = false;
            break;
    }

    result.meter = meter;
    if (meter == StandingMeter::Suspicion && event.systemId == 0) result.ignored = true;
    if (event.kind == StandingEventKind::ClaimEstablished && !result.ignored) {
        result.before = suspicion(event.systemId);
        const float floor = clampStanding(claimFloor);
        setSuspicion(event.systemId, std::max(result.before, floor));
        result.after = suspicion(event.systemId);
        result.appliedDelta = result.after - result.before;
        return result;
    }
    if (!std::isfinite(base) || base <= 0.0f || result.ignored) {
        result.before = meter == StandingMeter::Favor ? favor_ : suspicion(event.systemId);
        result.after = result.before;
        return result;
    }

    const float delta = discrete ? scaledDiscrete(base, scale) : base * scale;
    result.appliedDelta = std::max(0.0f, delta);
    if (meter == StandingMeter::Favor) {
        result.before = favor_;
        favor_ = clampStanding(favor_ + result.appliedDelta);
        result.after = favor_;
    } else {
        claimFloor = clampStanding(claimFloor);
        result.before = suspicion(event.systemId);
        const float next = std::max(claimFloor, result.before + result.appliedDelta);
        setSuspicion(event.systemId, next);
        result.after = suspicion(event.systemId);
    }
    result.appliedDelta = result.after - result.before;
    return result;
}

bool ImperialStandingLedger::decaySuspicion(ImperialSystemId systemId, float amount, float claimFloor) {
    if (systemId == 0 || !std::isfinite(amount) || amount <= 0.0f) return false;
    const float before = suspicion(systemId);
    // Inherited standing only decays above the notice threshold. A high claim
    // floor may keep the system above that threshold indefinitely.
    if (before <= 25.0f) return false;
    const float floor = std::max(25.0f, clampStanding(claimFloor));
    const float after = std::max(floor, before - amount);
    if (after == before) return false;
    setSuspicion(systemId, after);
    return true;
}

std::string ImperialStandingLedger::serializeState() const {
    std::ostringstream out;
    out << "ELYSIUM_IMPERIAL_STANDING 1\n";
    out << std::setprecision(9) << favor_ << ' ' << systems_.size() << '\n';
    for (const auto& record : systems_) out << record.systemId << ' ' << record.value << '\n';
    return out.str();
}

bool ImperialStandingLedger::restoreState(std::string_view text, std::string* error) {
    auto fail = [&](const char* message) { if (error) *error = message; return false; };
    std::istringstream in{std::string(text)};
    std::string magic;
    int schema{};
    if (!(in >> magic >> schema) || magic != "ELYSIUM_IMPERIAL_STANDING" || schema != 1)
        return fail("unsupported Imperial standing header");
    float favor{};
    std::size_t count{};
    if (!(in >> favor >> count) || !std::isfinite(favor) || count > 131072U)
        return fail("malformed Imperial standing state");
    std::vector<SystemSuspicionRecord> restored;
    restored.reserve(count);
    ImperialSystemId previous{};
    for (std::size_t i = 0; i < count; ++i) {
        SystemSuspicionRecord record{};
        if (!(in >> record.systemId >> record.value) || record.systemId == 0 || !std::isfinite(record.value) ||
            record.value < 0.0f || record.value > 100.0f || (i > 0 && record.systemId <= previous))
            return fail("invalid or unsorted system Suspicion record");
        previous = record.systemId;
        restored.push_back(record);
    }
    favor_ = clampStanding(favor);
    systems_ = std::move(restored);
    if (error) error->clear();
    return true;
}

const char* courtEnvoyName(CourtEnvoyId envoy) {
    switch (envoy) {
        case CourtEnvoyId::Elysomnion: return "Elysomnion";
        case CourtEnvoyId::SylpharaVoss: return "Sylphara Voss";
        case CourtEnvoyId::Sentinel: return "Sentinel";
        case CourtEnvoyId::Lillith: return "Lillith";
        case CourtEnvoyId::Aurelia: return "Aurelia";
        case CourtEnvoyId::Count: break;
    }
    return "Unknown";
}

ImperialClaimRegistry::ImperialClaimRegistry(std::uint64_t campaignSeed)
    : campaignSeed_(campaignSeed) {}

int ImperialClaimRegistry::slotLimit() const {
    int bonus = 0;
    for (bool value : befriended_) if (value) ++bonus;
    return std::min(8, 3 + bonus);
}

int ImperialClaimRegistry::activeClaimCount() const {
    return static_cast<int>(std::count_if(claims_.begin(), claims_.end(),
        [](const ImperialClaimRecord& claim) { return claim.active; }));
}

void ImperialClaimRegistry::setEnvoyBefriended(CourtEnvoyId envoy, bool value) {
    const auto index = envoyIndex(envoy);
    if (index < befriended_.size()) befriended_[index] = value;
}

bool ImperialClaimRegistry::envoyBefriended(CourtEnvoyId envoy) const {
    const auto index = envoyIndex(envoy);
    return index < befriended_.size() && befriended_[index];
}

ImperialStableId ImperialClaimRegistry::allocateClaimId() {
    for (;;) {
        const auto id = mix64(campaignSeed_ ^ kClaimLabel ^ nextClaimSerial_++);
        if (id != 0) return id;
    }
}

ClaimRegisterResult ImperialClaimRegistry::registerClaim(ImperialSystemId systemId,
                                                         ImperialPlanetId planetId,
                                                         ImperialStableId beaconStableId,
                                                         std::uint64_t structureVoxels,
                                                         ImperialStableId* outClaimId) {
    if (systemId == 0 || planetId == 0 || beaconStableId == 0) return ClaimRegisterResult::InvalidIdentity;
    if (isClaimed(planetId)) return ClaimRegisterResult::AlreadyClaimed;
    if (!canClaim()) return ClaimRegisterResult::SlotLimit;

    ImperialClaimRecord record{};
    record.claimId = allocateClaimId();
    record.systemId = systemId;
    record.planetId = planetId;
    record.beaconStableId = beaconStableId;
    record.structureVoxels = structureVoxels;
    record.active = true;
    claims_.push_back(record);
    std::sort(claims_.begin(), claims_.end(), [](const auto& a, const auto& b) { return a.claimId < b.claimId; });
    if (outClaimId) *outClaimId = record.claimId;
    return ClaimRegisterResult::Claimed;
}

bool ImperialClaimRegistry::abandonPlanet(ImperialPlanetId planetId) {
    for (auto& claim : claims_) {
        if (claim.planetId == planetId && claim.active) {
            claim.active = false;
            return true;
        }
    }
    return false;
}

bool ImperialClaimRegistry::strikeBeacon(ImperialStableId beaconStableId) {
    for (auto& claim : claims_) {
        if (claim.beaconStableId == beaconStableId && claim.active) {
            claim.active = false;
            return true;
        }
    }
    return false;
}

bool ImperialClaimRegistry::updateStructureVoxels(ImperialPlanetId planetId, std::uint64_t structureVoxels) {
    for (auto& claim : claims_) {
        if (claim.planetId == planetId && claim.active) {
            claim.structureVoxels = structureVoxels;
            return true;
        }
    }
    return false;
}

bool ImperialClaimRegistry::isClaimed(ImperialPlanetId planetId) const {
    return claimForPlanet(planetId) != nullptr;
}

const ImperialClaimRecord* ImperialClaimRegistry::claimForPlanet(ImperialPlanetId planetId) const {
    for (const auto& claim : claims_)
        if (claim.planetId == planetId && claim.active) return &claim;
    return nullptr;
}

float ImperialClaimRegistry::suspicionFloor(ImperialSystemId systemId) const {
    std::uint64_t structureVoxels = 0;
    bool any = false;
    for (const auto& claim : claims_) {
        if (!claim.active || claim.systemId != systemId) continue;
        any = true;
        const auto room = std::numeric_limits<std::uint64_t>::max() - structureVoxels;
        structureVoxels += std::min(room, claim.structureVoxels);
    }
    if (!any) return 0.0f;
    return std::min(70.0f, 8.0f + 2.0f * std::sqrt(static_cast<float>(structureVoxels) / 100.0f));
}

std::string ImperialClaimRegistry::serializeState() const {
    std::ostringstream out;
    out << "ELYSIUM_IMPERIAL_CLAIMS 1\n";
    out << campaignSeed_ << ' ' << nextClaimSerial_ << ' ' << claims_.size();
    for (bool value : befriended_) out << ' ' << (value ? 1 : 0);
    out << '\n';
    for (const auto& claim : claims_) {
        out << claim.claimId << ' ' << claim.systemId << ' ' << claim.planetId << ' '
            << claim.beaconStableId << ' ' << claim.structureVoxels << ' ' << (claim.active ? 1 : 0) << '\n';
    }
    return out.str();
}

bool ImperialClaimRegistry::restoreState(std::string_view text, std::string* error) {
    auto fail = [&](const char* message) { if (error) *error = message; return false; };
    std::istringstream in{std::string(text)};
    std::string magic;
    int schema{};
    if (!(in >> magic >> schema) || magic != "ELYSIUM_IMPERIAL_CLAIMS" || schema != 1)
        return fail("unsupported Imperial claim header");
    std::uint64_t seed{}, nextSerial{};
    std::size_t count{};
    if (!(in >> seed >> nextSerial >> count) || nextSerial == 0 || count > 64U)
        return fail("malformed Imperial claim state");
    std::array<bool, static_cast<std::size_t>(CourtEnvoyId::Count)> friends{};
    for (std::size_t i = 0; i < friends.size(); ++i) {
        int value{};
        if (!(in >> value) || (value != 0 && value != 1)) return fail("invalid Court friendship flag");
        friends[i] = value != 0;
    }
    std::vector<ImperialClaimRecord> restored;
    restored.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        ImperialClaimRecord claim{};
        int active{};
        if (!(in >> claim.claimId >> claim.systemId >> claim.planetId >> claim.beaconStableId >> claim.structureVoxels >> active) ||
            claim.claimId == 0 || claim.systemId == 0 || claim.planetId == 0 || claim.beaconStableId == 0 ||
            (active != 0 && active != 1))
            return fail("invalid Imperial claim record");
        claim.active = active != 0;
        restored.push_back(claim);
    }
    std::sort(restored.begin(), restored.end(), [](const auto& a, const auto& b) { return a.claimId < b.claimId; });
    if (std::adjacent_find(restored.begin(), restored.end(), [](const auto& a, const auto& b) { return a.claimId == b.claimId; }) != restored.end())
        return fail("duplicate stable claim ID");
    // One active claim per planet and one active beacon identity.
    for (std::size_t i = 0; i < restored.size(); ++i) for (std::size_t j = i + 1; j < restored.size(); ++j) {
        if (restored[i].active && restored[j].active &&
            (restored[i].planetId == restored[j].planetId || restored[i].beaconStableId == restored[j].beaconStableId))
            return fail("conflicting active Imperial claim records");
    }
    campaignSeed_ = seed;
    nextClaimSerial_ = nextSerial;
    befriended_ = friends;
    claims_ = std::move(restored);
    if (activeClaimCount() > slotLimit()) return fail("active claim count exceeds restored slot limit");
    if (error) error->clear();
    return true;
}

ImperialFileKnowledge::ImperialFileKnowledge(std::uint64_t campaignSeed)
    : campaignSeed_(campaignSeed) {}

ImperialStableId ImperialFileKnowledge::allocateFactId() {
    for (;;) {
        const auto id = mix64(campaignSeed_ ^ kFactLabel ^ nextFactSerial_++);
        if (id != 0) return id;
    }
}

ImperialStableId ImperialFileKnowledge::learn(ImperialSystemId systemId,
                                              ImperialFactKind kind,
                                              ImperialStableId sourceStableId,
                                              float confidence) {
    if (systemId == 0) return 0;
    ImperialFactRecord record{};
    record.factId = allocateFactId();
    record.systemId = systemId;
    record.kind = kind;
    record.sourceStableId = sourceStableId;
    record.confidence = std::clamp(std::isfinite(confidence) ? confidence : 0.0f, 0.0f, 1.0f);
    record.active = true;
    facts_.push_back(record);
    return record.factId;
}

bool ImperialFileKnowledge::suppress(ImperialStableId factId) {
    for (auto& fact : facts_) {
        if (fact.factId == factId && fact.active) {
            fact.active = false;
            return true;
        }
    }
    return false;
}

bool ImperialFileKnowledge::knows(ImperialSystemId systemId, ImperialFactKind kind) const {
    return std::any_of(facts_.begin(), facts_.end(), [&](const ImperialFactRecord& fact) {
        return fact.active && fact.systemId == systemId && fact.kind == kind && fact.confidence > 0.0f;
    });
}

std::size_t ImperialFileKnowledge::activeFactCount(ImperialSystemId systemId) const {
    return static_cast<std::size_t>(std::count_if(facts_.begin(), facts_.end(), [&](const ImperialFactRecord& fact) {
        return fact.active && fact.systemId == systemId;
    }));
}

std::string ImperialFileKnowledge::serializeState() const {
    std::ostringstream out;
    out << "ELYSIUM_IMPERIAL_FILE 1\n";
    out << campaignSeed_ << ' ' << nextFactSerial_ << ' ' << facts_.size() << '\n';
    out << std::setprecision(9);
    for (const auto& fact : facts_) {
        out << fact.factId << ' ' << fact.systemId << ' ' << static_cast<int>(fact.kind) << ' '
            << fact.sourceStableId << ' ' << fact.confidence << ' ' << (fact.active ? 1 : 0) << '\n';
    }
    return out.str();
}

bool ImperialFileKnowledge::restoreState(std::string_view text, std::string* error) {
    auto fail = [&](const char* message) { if (error) *error = message; return false; };
    std::istringstream in{std::string(text)};
    std::string magic;
    int schema{};
    if (!(in >> magic >> schema) || magic != "ELYSIUM_IMPERIAL_FILE" || schema != 1)
        return fail("unsupported Imperial FileKnowledge header");
    std::uint64_t seed{}, nextSerial{};
    std::size_t count{};
    if (!(in >> seed >> nextSerial >> count) || nextSerial == 0 || count > 1000000U)
        return fail("malformed Imperial FileKnowledge state");
    std::vector<ImperialFactRecord> restored;
    restored.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        ImperialFactRecord fact{};
        int kind{}, active{};
        if (!(in >> fact.factId >> fact.systemId >> kind >> fact.sourceStableId >> fact.confidence >> active) ||
            fact.factId == 0 || fact.systemId == 0 || kind < 0 || kind > static_cast<int>(ImperialFactKind::InspectionResult) ||
            !std::isfinite(fact.confidence) || fact.confidence < 0.0f || fact.confidence > 1.0f ||
            (active != 0 && active != 1))
            return fail("invalid Imperial FileKnowledge record");
        fact.kind = static_cast<ImperialFactKind>(kind);
        fact.active = active != 0;
        restored.push_back(fact);
    }
    auto ids = restored;
    std::sort(ids.begin(), ids.end(), [](const auto& a, const auto& b) { return a.factId < b.factId; });
    if (std::adjacent_find(ids.begin(), ids.end(), [](const auto& a, const auto& b) { return a.factId == b.factId; }) != ids.end())
        return fail("duplicate Imperial fact stable ID");
    campaignSeed_ = seed;
    nextFactSerial_ = nextSerial;
    facts_ = std::move(restored);
    if (error) error->clear();
    return true;
}

const CourtEnvoyDefinition& courtEnvoyDefinition(CourtEnvoyId envoy) {
    static const std::array<CourtEnvoyDefinition, static_cast<std::size_t>(CourtEnvoyId::Count)> definitions{{
        {CourtEnvoyId::Elysomnion, CourtReadGate::SuspicionHunted, StandingMeter::Suspicion},
        {CourtEnvoyId::SylpharaVoss, CourtReadGate::SuspicionNoted, StandingMeter::Suspicion},
        {CourtEnvoyId::Sentinel, CourtReadGate::Always, StandingMeter::Suspicion},
        {CourtEnvoyId::Lillith, CourtReadGate::FavorRecognised, StandingMeter::Favor},
        {CourtEnvoyId::Aurelia, CourtReadGate::FavorExalted, StandingMeter::Favor}
    }};
    const auto index = envoyIndex(envoy);
    return definitions[index < definitions.size() ? index : envoyIndex(CourtEnvoyId::Sentinel)];
}

CourtService::CourtService(std::uint64_t campaignSeed, CourtTuning tuning)
    : campaignSeed_(campaignSeed), tuning_(tuning) {
    tuning_.considerationSeconds = std::max(0.001f, tuning_.considerationSeconds);
    tuning_.considerationChance = std::clamp(tuning_.considerationChance, 0.0f, 1.0f);
    tuning_.visitSeconds = std::max(0.0f, tuning_.visitSeconds);
    tuning_.maximumRewardTier = std::clamp(tuning_.maximumRewardTier, 0, 3);
}

bool CourtService::eligible(CourtEnvoyId envoy, const ImperialStandingLedger& standing) const {
    const auto& definition = courtEnvoyDefinition(envoy);
    switch (definition.gate) {
        case CourtReadGate::Always: return true;
        case CourtReadGate::FavorRecognised:
            return ImperialStandingLedger::bandFor(standing.favor()) >= ImperialAttentionBand::Noted;
        case CourtReadGate::FavorExalted:
            return ImperialStandingLedger::bandFor(standing.favor()) >= ImperialAttentionBand::Hunted;
        case CourtReadGate::SuspicionNoted:
            return ImperialStandingLedger::bandFor(standing.maxSuspicion()) >= ImperialAttentionBand::Noted;
        case CourtReadGate::SuspicionHunted:
            return ImperialStandingLedger::bandFor(standing.maxSuspicion()) >= ImperialAttentionBand::Hunted;
    }
    return false;
}

std::vector<CourtEnvoyId> CourtService::eligibleEnvoys(const ImperialStandingLedger& standing) const {
    std::vector<CourtEnvoyId> out;
    for (std::size_t i = 0; i < static_cast<std::size_t>(CourtEnvoyId::Count); ++i) {
        const auto envoy = static_cast<CourtEnvoyId>(i);
        if (eligible(envoy, standing)) out.push_back(envoy);
    }
    return out;
}

ImperialStableId CourtService::allocateVisitId() {
    for (;;) {
        const auto id = mix64(campaignSeed_ ^ kVisitLabel ^ nextVisitSerial_++);
        if (id != 0) return id;
    }
}

ImperialStableId CourtService::allocateOfferId() {
    for (;;) {
        const auto id = mix64(campaignSeed_ ^ kOfferLabel ^ nextOfferSerial_++);
        if (id != 0) return id;
    }
}

CourtOffer CourtService::rollOffer(CourtEnvoyId envoy,
                                    const ImperialStandingLedger& standing,
                                    std::uint32_t revision) {
    CourtOffer offer{};
    offer.offerId = allocateOfferId();
    offer.revision = revision;
    const auto& definition = courtEnvoyDefinition(envoy);
    const float readValue = definition.reads == StandingMeter::Favor ? standing.favor() : standing.maxSuspicion();
    offer.rewardTier = std::min(tuning_.maximumRewardTier,
        static_cast<int>(ImperialStandingLedger::bandFor(readValue)));
    offer.tributeUnits = 1 + offer.rewardTier;
    // Construction commissions are a high-Favor outlet in the Fourth Edition.
    // The exact roll rate is intentionally non-load-bearing here; the portable
    // service only guarantees deterministic selection and persistence.
    const float commissionRoll = hash01(offer.offerId, static_cast<int>(envoy), offer.rewardTier,
                                        static_cast<int>(revision), 0x434F4D4DULL);
    offer.kind = standing.favor() >= 50.0f && commissionRoll < 0.35f
        ? CourtOfferKind::ConstructionCommission
        : CourtOfferKind::Exchange;
    return offer;
}

bool CourtService::summon(CourtEnvoyId envoy,
                          ImperialSystemId hostSystemId,
                          const ImperialStandingLedger& standing,
                          bool handPlaced) {
    if (activeVisit_ || hostSystemId == 0 || !eligible(envoy, standing)) return false;
    CourtVisit visit{};
    visit.visitId = allocateVisitId();
    visit.envoy = envoy;
    visit.hostSystemId = hostSystemId;
    visit.secondsRemaining = tuning_.visitSeconds;
    visit.handPlaced = handPlaced;
    visit.offer = rollOffer(envoy, standing, 0);
    activeVisit_ = visit;
    return true;
}

std::optional<CourtVisit> CourtService::update(float dt,
                                               ImperialSystemId hostSystemId,
                                               const ImperialStandingLedger& standing) {
    dt = std::max(0.0f, std::isfinite(dt) ? dt : 0.0f);
    if (activeVisit_) {
        if (!activeVisit_->handPlaced) {
            activeVisit_->secondsRemaining = std::max(0.0f, activeVisit_->secondsRemaining - dt);
            if (activeVisit_->secondsRemaining <= 0.0f) activeVisit_.reset();
        }
        return activeVisit_;
    }
    if (hostSystemId == 0) return std::nullopt;

    considerationAccumulator_ += dt;
    while (considerationAccumulator_ >= tuning_.considerationSeconds && !activeVisit_) {
        considerationAccumulator_ -= tuning_.considerationSeconds;
        const auto index = considerationIndex_++;
        const float roll = hash01(campaignSeed_ ^ index,
                                  static_cast<int>(hostSystemId & 0x7FFFFFFFU),
                                  static_cast<int>((hostSystemId >> 32U) & 0x7FFFFFFFU),
                                  static_cast<int>(index & 0x7FFFFFFFU), 0x5649534954ULL);
        if (roll > tuning_.considerationChance) continue;
        const auto candidates = eligibleEnvoys(standing);
        if (candidates.empty()) continue;

        // Preserve the inherited intent that visits lean toward whichever meter
        // is further advanced, while still filtering to envoys that would deal.
        std::vector<CourtEnvoyId> preferred;
        const bool favorLeads = standing.favor() > standing.maxSuspicion();
        for (const auto envoy : candidates) {
            if ((courtEnvoyDefinition(envoy).reads == StandingMeter::Favor) == favorLeads)
                preferred.push_back(envoy);
        }
        const auto& pool = preferred.empty() ? candidates : preferred;
        const auto choiceHash = mix64(campaignSeed_ ^ hostSystemId ^ index ^ kVisitLabel);
        const auto envoy = pool[static_cast<std::size_t>(choiceHash % pool.size())];
        summon(envoy, hostSystemId, standing, false);
    }
    return activeVisit_;
}

CourtTradeResult CourtService::acceptCurrentOffer(ImperialStandingLedger& standing) {
    CourtTradeResult result{};
    if (!activeVisit_) return result;
    result.accepted = true;
    result.previousOffer = activeVisit_->offer;
    const auto& definition = courtEnvoyDefinition(activeVisit_->envoy);
    StandingEvent event{};
    event.kind = StandingEventKind::CourtTribute;
    event.systemId = activeVisit_->hostSystemId;
    event.courtMeter = definition.reads;
    result.standing = standing.apply(event);
    activeVisit_->offer = rollOffer(activeVisit_->envoy, standing, activeVisit_->offer.revision + 1U);
    result.nextOffer = activeVisit_->offer;
    return result;
}

void CourtService::dismiss() {
    activeVisit_.reset();
}

void CourtService::setBefriended(CourtEnvoyId envoy, bool value) {
    const auto index = envoyIndex(envoy);
    if (index < befriended_.size()) befriended_[index] = value;
}

bool CourtService::befriended(CourtEnvoyId envoy) const {
    const auto index = envoyIndex(envoy);
    return index < befriended_.size() && befriended_[index];
}

std::string CourtService::serializeState() const {
    std::ostringstream out;
    out << "ELYSIUM_COURT_STATE 1\n";
    out << campaignSeed_ << ' ' << nextVisitSerial_ << ' ' << nextOfferSerial_ << ' ' << considerationIndex_ << ' '
        << std::setprecision(9) << considerationAccumulator_;
    for (bool value : befriended_) out << ' ' << (value ? 1 : 0);
    out << ' ' << (activeVisit_ ? 1 : 0) << '\n';
    if (activeVisit_) {
        out << activeVisit_->visitId << ' ' << static_cast<int>(activeVisit_->envoy) << ' ' << activeVisit_->hostSystemId << ' '
            << activeVisit_->secondsRemaining << ' ' << (activeVisit_->handPlaced ? 1 : 0) << ' '
            << activeVisit_->offer.offerId << ' ' << static_cast<int>(activeVisit_->offer.kind) << ' '
            << activeVisit_->offer.rewardTier << ' ' << activeVisit_->offer.tributeUnits << ' '
            << activeVisit_->offer.revision << '\n';
    }
    return out.str();
}

bool CourtService::restoreState(std::string_view text, std::string* error) {
    auto fail = [&](const char* message) { if (error) *error = message; return false; };
    std::istringstream in{std::string(text)};
    std::string magic;
    int schema{};
    if (!(in >> magic >> schema) || magic != "ELYSIUM_COURT_STATE" || schema != 1)
        return fail("unsupported Court state header");
    std::uint64_t seed{}, nextVisit{}, nextOffer{}, consideration{};
    float accumulator{};
    if (!(in >> seed >> nextVisit >> nextOffer >> consideration >> accumulator) || nextVisit == 0 || nextOffer == 0 ||
        !std::isfinite(accumulator) || accumulator < 0.0f || accumulator >= tuning_.considerationSeconds)
        return fail("malformed Court state");
    std::array<bool, static_cast<std::size_t>(CourtEnvoyId::Count)> friends{};
    for (std::size_t i = 0; i < friends.size(); ++i) {
        int value{};
        if (!(in >> value) || (value != 0 && value != 1)) return fail("invalid Court friendship state");
        friends[i] = value != 0;
    }
    int hasVisit{};
    if (!(in >> hasVisit) || (hasVisit != 0 && hasVisit != 1)) return fail("invalid Court active-visit flag");
    std::optional<CourtVisit> restored;
    if (hasVisit) {
        CourtVisit visit{};
        int envoy{}, handPlaced{}, kind{};
        if (!(in >> visit.visitId >> envoy >> visit.hostSystemId >> visit.secondsRemaining >> handPlaced >>
              visit.offer.offerId >> kind >> visit.offer.rewardTier >> visit.offer.tributeUnits >> visit.offer.revision) ||
            visit.visitId == 0 || visit.offer.offerId == 0 || visit.hostSystemId == 0 ||
            !validEnumInt(envoy, CourtEnvoyId::Count) || kind < 0 || kind > static_cast<int>(CourtOfferKind::ConstructionCommission) ||
            !std::isfinite(visit.secondsRemaining) || visit.secondsRemaining < 0.0f ||
            (handPlaced != 0 && handPlaced != 1) || visit.offer.rewardTier < 0 || visit.offer.rewardTier > 3 ||
            visit.offer.tributeUnits < 1)
            return fail("invalid active Court visit");
        visit.envoy = static_cast<CourtEnvoyId>(envoy);
        visit.handPlaced = handPlaced != 0;
        visit.offer.kind = static_cast<CourtOfferKind>(kind);
        restored = visit;
    }
    campaignSeed_ = seed;
    nextVisitSerial_ = nextVisit;
    nextOfferSerial_ = nextOffer;
    considerationIndex_ = consideration;
    considerationAccumulator_ = accumulator;
    befriended_ = friends;
    activeVisit_ = restored;
    if (error) error->clear();
    return true;
}

ImperialBureaucracyDecision evaluateImperialBureaucracy(
    ImperialSystemId systemId,
    const ImperialStandingLedger& standing,
    const ImperialFileKnowledge& file,
    bool carryingKnownContraband,
    bool activeWarrant) {
    ImperialBureaucracyDecision result{};
    result.knownFactCount = static_cast<int>(std::min<std::size_t>(
        file.activeFactCount(systemId), static_cast<std::size_t>(std::numeric_limits<int>::max())));
    const auto band = ImperialStandingLedger::bandFor(standing.suspicion(systemId));
    const bool knownClaim = file.knows(systemId, ImperialFactKind::Claim);
    const bool knownExtraction = file.knows(systemId, ImperialFactKind::HighVolumeExtraction);
    result.inspectionRecommended = carryingKnownContraband || activeWarrant || knownClaim || knownExtraction ||
                                   band >= ImperialAttentionBand::Noted;
    result.routeInterdictionActive = band >= ImperialAttentionBand::Hunted;

    if (result.routeInterdictionActive) result.disposition = CustomsDisposition::Interdict;
    else if (activeWarrant) result.disposition = CustomsDisposition::DetainForWarrant;
    else if (result.inspectionRecommended) result.disposition = CustomsDisposition::Inspect;
    else result.disposition = CustomsDisposition::Clear;
    return result;
}

ImperialCampaignState::ImperialCampaignState(std::uint64_t campaignSeed, CourtTuning courtTuning)
    : claims(campaignSeed), fileKnowledge(campaignSeed), court(campaignSeed, courtTuning), campaignSeed_(campaignSeed) {}

void ImperialCampaignState::setEnforcementState(ImperialSystemId systemId, std::string directorState) {
    if (systemId == 0) return;
    auto it = std::lower_bound(enforcement_.begin(), enforcement_.end(), systemId,
        [](const ImperialEnforcementRecord& record, ImperialSystemId id) { return record.systemId < id; });
    if (directorState.empty()) {
        if (it != enforcement_.end() && it->systemId == systemId) enforcement_.erase(it);
        return;
    }
    if (it != enforcement_.end() && it->systemId == systemId) it->directorState = std::move(directorState);
    else enforcement_.insert(it, ImperialEnforcementRecord{systemId, std::move(directorState)});
}

const std::string* ImperialCampaignState::enforcementState(ImperialSystemId systemId) const {
    const auto it = std::lower_bound(enforcement_.begin(), enforcement_.end(), systemId,
        [](const ImperialEnforcementRecord& record, ImperialSystemId id) { return record.systemId < id; });
    return it != enforcement_.end() && it->systemId == systemId ? &it->directorState : nullptr;
}

bool ImperialCampaignState::eraseEnforcementState(ImperialSystemId systemId) {
    auto it = std::lower_bound(enforcement_.begin(), enforcement_.end(), systemId,
        [](const ImperialEnforcementRecord& record, ImperialSystemId id) { return record.systemId < id; });
    if (it == enforcement_.end() || it->systemId != systemId) return false;
    enforcement_.erase(it);
    return true;
}

void ImperialCampaignState::syncClaimSlotsFromCourt() {
    for (std::size_t i = 0; i < static_cast<std::size_t>(CourtEnvoyId::Count); ++i) {
        const auto envoy = static_cast<CourtEnvoyId>(i);
        claims.setEnvoyBefriended(envoy, court.befriended(envoy));
    }
}

std::string ImperialCampaignState::serializeState() const {
    std::ostringstream out;
    out << "ELYSIUM_IMPERIAL_CAMPAIGN 1\n";
    out << campaignSeed_ << ' ' << enforcement_.size() << '\n';
    out << "standing " << hexEncode(standing.serializeState()) << '\n';
    out << "claims " << hexEncode(claims.serializeState()) << '\n';
    out << "file " << hexEncode(fileKnowledge.serializeState()) << '\n';
    out << "court " << hexEncode(court.serializeState()) << '\n';
    for (const auto& record : enforcement_)
        out << "enforcement " << record.systemId << ' ' << hexEncode(record.directorState) << '\n';
    return out.str();
}

bool ImperialCampaignState::restoreState(std::string_view text, std::string* error) {
    auto fail = [&](const std::string& message) { if (error) *error = message; return false; };
    std::istringstream in{std::string(text)};
    std::string magic;
    int schema{};
    if (!(in >> magic >> schema) || magic != "ELYSIUM_IMPERIAL_CAMPAIGN" || schema != 1)
        return fail("unsupported Imperial campaign header");
    std::uint64_t seed{};
    std::size_t enforcementCount{};
    if (!(in >> seed >> enforcementCount) || enforcementCount > 131072U)
        return fail("malformed Imperial campaign state");

    std::string label, encoded, decoded;
    ImperialStandingLedger restoredStanding;
    ImperialClaimRegistry restoredClaims(seed);
    ImperialFileKnowledge restoredFile(seed);
    CourtService restoredCourt(seed, court.tuning());

    if (!(in >> label >> encoded) || label != "standing" || !hexDecode(encoded, decoded))
        return fail("malformed Imperial standing envelope");
    std::string nestedError;
    if (!restoredStanding.restoreState(decoded, &nestedError)) return fail("standing: " + nestedError);

    if (!(in >> label >> encoded) || label != "claims" || !hexDecode(encoded, decoded))
        return fail("malformed Imperial claim envelope");
    if (!restoredClaims.restoreState(decoded, &nestedError)) return fail("claims: " + nestedError);

    if (!(in >> label >> encoded) || label != "file" || !hexDecode(encoded, decoded))
        return fail("malformed Imperial FileKnowledge envelope");
    if (!restoredFile.restoreState(decoded, &nestedError)) return fail("file: " + nestedError);

    if (!(in >> label >> encoded) || label != "court" || !hexDecode(encoded, decoded))
        return fail("malformed Court envelope");
    if (!restoredCourt.restoreState(decoded, &nestedError)) return fail("court: " + nestedError);

    std::vector<ImperialEnforcementRecord> restoredEnforcement;
    restoredEnforcement.reserve(enforcementCount);
    ImperialSystemId previous{};
    for (std::size_t i = 0; i < enforcementCount; ++i) {
        ImperialEnforcementRecord record{};
        if (!(in >> label >> record.systemId >> encoded) || label != "enforcement" || record.systemId == 0 ||
            (i > 0 && record.systemId <= previous) || !hexDecode(encoded, record.directorState))
            return fail("malformed or unsorted Imperial enforcement envelope");
        SurfaceSiegeDirector validation(seed ^ record.systemId);
        if (!validation.restoreState(record.directorState, &nestedError))
            return fail("enforcement: " + nestedError);
        previous = record.systemId;
        restoredEnforcement.push_back(std::move(record));
    }

    campaignSeed_ = seed;
    standing = std::move(restoredStanding);
    claims = std::move(restoredClaims);
    fileKnowledge = std::move(restoredFile);
    court = std::move(restoredCourt);
    enforcement_ = std::move(restoredEnforcement);
    if (error) error->clear();
    return true;
}

} // namespace elysium
