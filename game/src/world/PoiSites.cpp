// Intended function: imported world implementation for PoiSites; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/PoiSites.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <set>
#include <sstream>
#include <tuple>
#include <utility>

namespace elysium {
namespace {

constexpr std::uint64_t kSiteLabel = 0x534954455F504F49ULL;
constexpr std::uint64_t kLootLabel = 0x4C4F4F545F504F49ULL;
constexpr std::uint64_t kEncounterLabel = 0x454E435F504F495FULL;
constexpr std::uint64_t kObjectLabel = 0x4F424A5F504F495FULL;
constexpr std::uint64_t kHistoryLabel = 0x484953545F504F49ULL;

SiteKitDefinition kit(SiteFamily family, SiteDomain domain, SiteFaction faction,
                      SiteArchitecture architecture, SettlementScale settlement,
                      std::initializer_list<SiteModuleSpec> modules) {
    return {family, domain, faction, architecture, settlement, std::vector<SiteModuleSpec>(modules)};
}

SiteModuleSpec req(SiteModuleKind kind) { return {kind, true, 255}; }
SiteModuleSpec opt(SiteModuleKind kind, std::uint8_t weight = 128) { return {kind, false, weight}; }

std::uint64_t hashAddress(std::uint64_t seed, const SurfaceCellAddress& a, std::uint64_t label) {
    std::uint64_t h = mix64(seed ^ label ^ static_cast<std::uint64_t>(a.face));
    h = mix64(h ^ static_cast<std::uint64_t>(static_cast<std::uint32_t>(a.u)));
    h = mix64(h ^ (static_cast<std::uint64_t>(static_cast<std::uint32_t>(a.v)) << 1U));
    h = mix64(h ^ (static_cast<std::uint64_t>(static_cast<std::uint32_t>(a.radial)) << 2U));
    return h;
}

int floorDiv(int value, int divisor) {
    int q = value / divisor;
    int r = value % divisor;
    if (r < 0) --q;
    return q;
}

int clamp100(int v) { return std::clamp(v, 0, 100); }

std::uint8_t satAdd(std::uint8_t a, std::uint8_t b) {
    return static_cast<std::uint8_t>(std::min(100, static_cast<int>(a) + static_cast<int>(b)));
}

bool isSurfaceLike(SiteDomain domain) {
    return domain == SiteDomain::Surface || domain == SiteDomain::Subsurface || domain == SiteDomain::Anomalous;
}

std::uint64_t stateFingerprint(const SitePersistentState& s) {
    std::uint64_t h = mix64(s.stableId ^ static_cast<std::uint64_t>(s.lifecycle));
    h = mix64(h ^ static_cast<std::uint64_t>(s.owner));
    h = mix64(h ^ static_cast<std::uint64_t>(static_cast<std::uint32_t>(s.populationOverride)));
    h = mix64(h ^ static_cast<std::uint64_t>(s.addedDamage));
    h = mix64(h ^ static_cast<std::uint64_t>(s.contamination));
    h = mix64(h ^ static_cast<std::uint64_t>(s.discovered ? 1 : 0));
    h = mix64(h ^ s.historicalOwnerId);
    for (const auto& [id, state] : s.objectStates) h = mix64(h ^ id ^ static_cast<std::uint64_t>(state));
    return h;
}

} // namespace

SiteCatalog::SiteCatalog() {
    // Every family from Third Edition Part 18 is represented by semantic modules.
    // Optional module inclusion is deterministic; geometry remains an authored asset concern.
    kits_ = {
        kit(SiteFamily::SupplyCache, SiteDomain::Surface, SiteFaction::Frontier, SiteArchitecture::Frontier, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Storage), opt(SiteModuleKind::Security,70)}),
        kit(SiteFamily::CrashedShip, SiteDomain::Surface, SiteFaction::None, SiteArchitecture::Industrial, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Salvage), opt(SiteModuleKind::Power,140), opt(SiteModuleKind::Archive,90)}),
        kit(SiteFamily::SurveyOutpost, SiteDomain::Surface, SiteFaction::Frontier, SiteArchitecture::Frontier, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Sensor), req(SiteModuleKind::Archive), opt(SiteModuleKind::Power,180)}),
        kit(SiteFamily::UnswornCamp, SiteDomain::Surface, SiteFaction::Unsworn, SiteArchitecture::Unsworn, SettlementScale::Camp,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Habitation), req(SiteModuleKind::Trade), opt(SiteModuleKind::Workshop,150), opt(SiteModuleKind::Medical,90)}),
        kit(SiteFamily::MiningRig, SiteDomain::Surface, SiteFaction::Frontier, SiteArchitecture::Industrial, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Workshop), req(SiteModuleKind::Power), req(SiteModuleKind::Storage), opt(SiteModuleKind::Security,120)}),
        kit(SiteFamily::Ruin, SiteDomain::Surface, SiteFaction::Ancient, SiteArchitecture::Ancient, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Archive), opt(SiteModuleKind::Security,150), opt(SiteModuleKind::Salvage,180)}),
        kit(SiteFamily::Monolith, SiteDomain::Surface, SiteFaction::Ancient, SiteArchitecture::Ancient, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Archive)}),
        kit(SiteFamily::RiftAnomaly, SiteDomain::Anomalous, SiteFaction::None, SiteArchitecture::Rift, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::RiftPortal), opt(SiteModuleKind::Research,150)}),
        kit(SiteFamily::ImperialWaystation, SiteDomain::Surface, SiteFaction::Imperial, SiteArchitecture::Imperial, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Command), req(SiteModuleKind::Security), req(SiteModuleKind::Power), opt(SiteModuleKind::Storage,180)}),
        kit(SiteFamily::AbandonedFarm, SiteDomain::Surface, SiteFaction::Frontier, SiteArchitecture::Frontier, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Agriculture), req(SiteModuleKind::Storage), opt(SiteModuleKind::Habitation,180)}),
        kit(SiteFamily::WeatherStation, SiteDomain::Surface, SiteFaction::Frontier, SiteArchitecture::Industrial, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Sensor), req(SiteModuleKind::Power), opt(SiteModuleKind::Archive,160)}),
        kit(SiteFamily::SubsurfaceVault, SiteDomain::Subsurface, SiteFaction::Imperial, SiteArchitecture::Imperial, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Storage), req(SiteModuleKind::Security), opt(SiteModuleKind::Archive,170)}),
        kit(SiteFamily::BuriedReactor, SiteDomain::Subsurface, SiteFaction::None, SiteArchitecture::Industrial, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Reactor), req(SiteModuleKind::Utility), opt(SiteModuleKind::Security,130), opt(SiteModuleKind::Salvage,190)}),
        kit(SiteFamily::ResearchHabitat, SiteDomain::Surface, SiteFaction::Frontier, SiteArchitecture::Frontier, SettlementScale::Hamlet,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Habitation), req(SiteModuleKind::Research), req(SiteModuleKind::Power), opt(SiteModuleKind::Medical,170)}),
        kit(SiteFamily::PlanetaryRelay, SiteDomain::Surface, SiteFaction::Imperial, SiteArchitecture::Imperial, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Sensor), req(SiteModuleKind::Command), req(SiteModuleKind::Power), opt(SiteModuleKind::Security,180)}),
        kit(SiteFamily::SmugglerDen, SiteDomain::Subsurface, SiteFaction::Unsworn, SiteArchitecture::Unsworn, SettlementScale::Camp,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Trade), req(SiteModuleKind::Storage), opt(SiteModuleKind::Security,170), opt(SiteModuleKind::Hangar,100)}),
        kit(SiteFamily::AncientObservatory, SiteDomain::Surface, SiteFaction::Ancient, SiteArchitecture::Ancient, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Sensor), req(SiteModuleKind::Archive), opt(SiteModuleKind::Research,170)}),
        kit(SiteFamily::GeothermalPlant, SiteDomain::Surface, SiteFaction::Frontier, SiteArchitecture::Industrial, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Power), req(SiteModuleKind::Utility), opt(SiteModuleKind::Workshop,190)}),
        kit(SiteFamily::FloodedComplex, SiteDomain::Subsurface, SiteFaction::None, SiteArchitecture::Industrial, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Utility), req(SiteModuleKind::Salvage), opt(SiteModuleKind::Research,130)}),
        kit(SiteFamily::OrbitalDebrisFall, SiteDomain::Surface, SiteFaction::None, SiteArchitecture::Industrial, SettlementScale::None,
            {req(SiteModuleKind::Salvage), opt(SiteModuleKind::Storage,180), opt(SiteModuleKind::Archive,100)}),
        kit(SiteFamily::TradingPost, SiteDomain::Orbit, SiteFaction::Frontier, SiteArchitecture::Industrial, SettlementScale::FrontierTown,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Trade), req(SiteModuleKind::Hangar), req(SiteModuleKind::Habitation), req(SiteModuleKind::Power), opt(SiteModuleKind::Medical,140)}),
        kit(SiteFamily::ImperialStation, SiteDomain::Orbit, SiteFaction::Imperial, SiteArchitecture::Imperial, SettlementScale::ImperialEnclave,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Command), req(SiteModuleKind::Security), req(SiteModuleKind::Hangar), req(SiteModuleKind::Trade), req(SiteModuleKind::Power)}),
        kit(SiteFamily::UnswornHaven, SiteDomain::Orbit, SiteFaction::Unsworn, SiteArchitecture::Unsworn, SettlementScale::UnswornHaven,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Trade), req(SiteModuleKind::Hangar), req(SiteModuleKind::Salvage), req(SiteModuleKind::Habitation), opt(SiteModuleKind::Archive,130)}),
        kit(SiteFamily::DerelictFreighter, SiteDomain::Space, SiteFaction::None, SiteArchitecture::Industrial, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Salvage), req(SiteModuleKind::Power), req(SiteModuleKind::Security), opt(SiteModuleKind::Archive,180)}),
        kit(SiteFamily::DataBuoy, SiteDomain::Space, SiteFaction::None, SiteArchitecture::Industrial, SettlementScale::None,
            {req(SiteModuleKind::Sensor), req(SiteModuleKind::Archive), opt(SiteModuleKind::Power,180)}),
        kit(SiteFamily::AsteroidRefinery, SiteDomain::Space, SiteFaction::Frontier, SiteArchitecture::Industrial, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Workshop), req(SiteModuleKind::Storage), req(SiteModuleKind::Power), opt(SiteModuleKind::Hangar,180)}),
        kit(SiteFamily::SilentColony, SiteDomain::Surface, SiteFaction::None, SiteArchitecture::Frontier, SettlementScale::FrontierTown,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Habitation), req(SiteModuleKind::Medical), req(SiteModuleKind::Archive), opt(SiteModuleKind::Security,170)}),
        kit(SiteFamily::SiegeRuin, SiteDomain::Surface, SiteFaction::None, SiteArchitecture::Frontier, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Security), req(SiteModuleKind::Salvage), opt(SiteModuleKind::Power,110), opt(SiteModuleKind::Command,100)}),
        kit(SiteFamily::AnomalyLabyrinth, SiteDomain::Anomalous, SiteFaction::None, SiteArchitecture::Rift, SettlementScale::None,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::RiftPortal), req(SiteModuleKind::Research), opt(SiteModuleKind::Archive,170)}),
        kit(SiteFamily::CourtAnchorage, SiteDomain::Orbit, SiteFaction::Imperial, SiteArchitecture::Imperial, SettlementScale::ImperialEnclave,
            {req(SiteModuleKind::Entry), req(SiteModuleKind::Command), req(SiteModuleKind::Hangar), req(SiteModuleKind::Security), req(SiteModuleKind::Trade), opt(SiteModuleKind::Archive,210)}),
    };
    std::sort(kits_.begin(), kits_.end(), [](const auto& a, const auto& b) { return a.family < b.family; });
}

const SiteKitDefinition* SiteCatalog::find(SiteFamily family) const {
    const auto it = std::lower_bound(kits_.begin(), kits_.end(), family,
        [](const SiteKitDefinition& a, SiteFamily b) { return a.family < b; });
    return it != kits_.end() && it->family == family ? &*it : nullptr;
}

SettlementProfile settlementProfile(SettlementScale scale, SiteFaction faction, std::uint64_t seed) {
    SettlementProfile p{};
    p.scale = scale;
    p.faction = faction;
    switch (scale) {
        case SettlementScale::None: return p;
        case SettlementScale::Camp:
            p.minPopulation = 3; p.maxPopulation = 8;
            p.services = SettlementServiceTrade | SettlementServiceRumors | SettlementServiceDirections | SettlementServiceContracts;
            break;
        case SettlementScale::Hamlet:
            p.minPopulation = 8; p.maxPopulation = 25;
            p.services = SettlementServiceTrade | SettlementServiceRumors | SettlementServiceDirections | SettlementServiceContracts |
                         SettlementServiceFood | SettlementServiceRepair | SettlementServiceSpecialist;
            break;
        case SettlementScale::FrontierTown:
            p.minPopulation = 25; p.maxPopulation = 80;
            p.services = SettlementServiceTrade | SettlementServiceRumors | SettlementServiceDirections | SettlementServiceContracts |
                         SettlementServiceFood | SettlementServiceRepair | SettlementServiceSpecialist | SettlementServiceMarket |
                         SettlementServiceShipPad | SettlementServiceFaction;
            break;
        case SettlementScale::ImperialEnclave:
            p.minPopulation = 20; p.maxPopulation = 120;
            p.services = SettlementServiceTrade | SettlementServiceContracts | SettlementServiceRepair | SettlementServiceMarket |
                         SettlementServiceShipPad | SettlementServiceFaction | SettlementServiceInspection | SettlementServiceHousing;
            break;
        case SettlementScale::UnswornHaven:
            p.minPopulation = 15; p.maxPopulation = 100;
            p.services = SettlementServiceTrade | SettlementServiceRumors | SettlementServiceDirections | SettlementServiceContracts |
                         SettlementServiceRepair | SettlementServiceMarket | SettlementServiceShipPad | SettlementServiceCharts |
                         SettlementServiceSalvage | SettlementServiceBlackMarket | SettlementServiceHousing;
            break;
        case SettlementScale::PlayerOutpost:
            p.minPopulation = 0; p.maxPopulation = 256;
            p.services = SettlementServiceTrade | SettlementServiceContracts | SettlementServiceRepair | SettlementServiceShipPad |
                         SettlementServiceHousing | SettlementServiceAutomation;
            break;
    }
    const int span = std::max(1, p.maxPopulation - p.minPopulation + 1);
    p.baselinePopulation = p.minPopulation + static_cast<int>(mix64(seed ^ 0x504F50554C415449ULL) % static_cast<std::uint64_t>(span));
    return p;
}

SiteGenerator::SiteGenerator(SiteCatalog catalog) : catalog_(std::move(catalog)) {}

SiteFamily SiteGenerator::chooseSurfaceFamily(std::uint64_t h) const {
    static constexpr std::array<SiteFamily, 20> families = {
        SiteFamily::SupplyCache, SiteFamily::CrashedShip, SiteFamily::SurveyOutpost, SiteFamily::UnswornCamp,
        SiteFamily::MiningRig, SiteFamily::Ruin, SiteFamily::Monolith, SiteFamily::RiftAnomaly,
        SiteFamily::ImperialWaystation, SiteFamily::AbandonedFarm, SiteFamily::WeatherStation, SiteFamily::SubsurfaceVault,
        SiteFamily::BuriedReactor, SiteFamily::ResearchHabitat, SiteFamily::PlanetaryRelay, SiteFamily::SmugglerDen,
        SiteFamily::AncientObservatory, SiteFamily::GeothermalPlant, SiteFamily::FloodedComplex, SiteFamily::OrbitalDebrisFall,
    };
    return families[static_cast<std::size_t>(h % families.size())];
}

SiteDescriptor SiteGenerator::assemble(std::uint64_t worldSeed, SiteFamily family,
                                       std::optional<SurfaceCellAddress> anchor,
                                       std::uint32_t strategicSlot) const {
    const auto* kitDef = catalog_.find(family);
    if (!kitDef) return {};
    return assembleFromKit(worldSeed, *kitDef, anchor, strategicSlot);
}

SiteDescriptor SiteGenerator::assembleFromKit(std::uint64_t worldSeed, const SiteKitDefinition& kitDef,
                                              std::optional<SurfaceCellAddress> anchor,
                                              std::uint32_t strategicSlot) const {
    std::uint64_t identity = mix64(worldSeed ^ kSiteLabel ^ static_cast<std::uint64_t>(kitDef.family));
    if (anchor) identity = hashAddress(identity, *anchor, kSiteLabel);
    else identity = mix64(identity ^ (static_cast<std::uint64_t>(strategicSlot) << 17U));
    if (identity == 0) identity = 1;

    SiteDescriptor d{};
    d.stableId = identity;
    d.family = kitDef.family;
    d.domain = kitDef.domain;
    d.owner = kitDef.defaultFaction;
    d.architecture = kitDef.architecture;
    d.lifecycle = SiteLifecycleState::GeneratedUnknown;
    d.anchor = anchor;
    d.strategicSlot = strategicSlot;
    d.quarterTurns = static_cast<std::uint8_t>(mix64(identity ^ 0x4F5249454E54ULL) & 3ULL);
    d.damage = static_cast<std::uint8_t>((mix64(identity ^ 0x44414D414745ULL) >> 8U) % 31ULL);
    if (kitDef.family == SiteFamily::Ruin || kitDef.family == SiteFamily::SiegeRuin || kitDef.family == SiteFamily::DerelictFreighter)
        d.damage = static_cast<std::uint8_t>(35 + ((mix64(identity ^ 0x5255494EULL) >> 12U) % 51ULL));
    d.contamination = static_cast<std::uint8_t>((mix64(identity ^ 0x434F4E54414DULL) >> 9U) % 16ULL);
    d.lootSeed = mix64(identity ^ kLootLabel);
    d.encounterSeed = mix64(identity ^ kEncounterLabel);
    d.historicalOwnerId = kitDef.defaultFaction == SiteFaction::None ? 0 : mix64(identity ^ 0x4F574E45525FULL);
    d.settlement = settlementProfile(kitDef.settlementScale, kitDef.defaultFaction, identity);

    std::size_t ordinal = 0;
    for (const auto& module : kitDef.modules) {
        const std::uint64_t mh = mix64(identity ^ kObjectLabel ^ (static_cast<std::uint64_t>(module.kind) << 16U) ^ ordinal);
        if (!module.required && static_cast<std::uint8_t>(mh & 0xFFULL) >= module.weight) {
            ++ordinal;
            continue;
        }
        SiteModuleInstance instance{};
        instance.stableObjectId = mh == 0 ? 1 : mh;
        instance.kind = module.kind;
        instance.variant = static_cast<std::uint8_t>((mh >> 16U) & 0x03ULL);
        instance.damage = static_cast<std::uint8_t>(clamp100(static_cast<int>(d.damage) + static_cast<int>((mh >> 24U) % 17ULL) - 8));
        d.modules.push_back(instance);
        ++ordinal;
    }
    std::sort(d.modules.begin(), d.modules.end(), [](const auto& a, const auto& b) { return a.stableObjectId < b.stableObjectId; });
    return d;
}

std::vector<SiteDescriptor> SiteGenerator::generateSurface(const PlanetSurface& planet,
                                                           const SiteSurfaceQuery& query,
                                                           const SiteSpacingPolicy& policy) const {
    std::vector<SiteDescriptor> out;
    if (policy.placementCellColumns <= 0 || policy.minSpacingColumns < 0 || policy.maxSitesPerQuery <= 0 ||
        query.uEnd <= query.uBegin || query.vEnd <= query.vBegin) return out;

    struct Candidate { int gridU{}; int gridV{}; SurfaceCellAddress anchor{}; std::uint64_t priority{}; SiteFamily family{}; };
    const int cs = policy.placementCellColumns;
    const int gu0 = floorDiv(query.uBegin, cs) - 1;
    const int gv0 = floorDiv(query.vBegin, cs) - 1;
    const int gu1 = floorDiv(query.uEnd - 1, cs) + 1;
    const int gv1 = floorDiv(query.vEnd - 1, cs) + 1;

    const auto makeCandidate = [&](int gu, int gv) -> std::optional<Candidate> {
        const std::uint64_t biomeLabel = 0x504F495F43414E44ULL ^ (static_cast<std::uint64_t>(planet.planetClass()) << 40U);
        const std::uint64_t base = hashCoords(planet.seed(), gu, gv, static_cast<int>(query.face), biomeLabel);
        if (static_cast<std::uint8_t>(base & 0xFFULL) >= policy.candidateChance) return std::nullopt;
        const int jitterU = static_cast<int>((base >> 8U) % static_cast<std::uint64_t>(cs));
        const int jitterV = static_cast<int>((base >> 24U) % static_cast<std::uint64_t>(cs));
        const int rawU = gu * cs + jitterU;
        const int rawV = gv * cs + jitterV;
        SurfaceCellAddress normalized = planet.normalize({query.face, rawU, rawV, PlanetSurface::ReferenceRadial});
        // Each face owns only candidates that project back to it. This avoids one
        // off-face placement being emitted by multiple face-local generators.
        if (normalized.face != query.face) return std::nullopt;
        normalized.radial = planet.surfaceRadial(normalized.face, normalized.u, normalized.v) + 1;
        if (!planet.radialInBounds(normalized.radial)) normalized.radial = planet.surfaceRadial(normalized.face, normalized.u, normalized.v);
        return Candidate{gu, gv, normalized, mix64(base ^ 0x5052494F52495459ULL), chooseSurfaceFamily(base >> 32U)};
    };

    for (int gv = gv0; gv <= gv1; ++gv) {
        for (int gu = gu0; gu <= gu1; ++gu) {
            auto candidate = makeCandidate(gu, gv);
            if (!candidate) continue;
            const auto& a = candidate->anchor;
            if (a.u < query.uBegin || a.u >= query.uEnd || a.v < query.vBegin || a.v >= query.vEnd) continue;

            bool losesSpacing = false;
            for (int ngv = gv - 1; ngv <= gv + 1 && !losesSpacing; ++ngv) {
                for (int ngu = gu - 1; ngu <= gu + 1; ++ngu) {
                    if (ngu == gu && ngv == gv) continue;
                    auto other = makeCandidate(ngu, ngv);
                    if (!other) continue;
                    const int du = other->anchor.u - a.u;
                    const int dv = other->anchor.v - a.v;
                    if (du * du + dv * dv >= policy.minSpacingColumns * policy.minSpacingColumns) continue;
                    const auto mineKey = std::tuple{candidate->priority, gu, gv};
                    const auto otherKey = std::tuple{other->priority, ngu, ngv};
                    if (otherKey < mineKey) { losesSpacing = true; break; }
                }
            }
            if (losesSpacing) continue;
            const auto* kitDef = catalog_.find(candidate->family);
            if (!kitDef || !isSurfaceLike(kitDef->domain)) continue;
            out.push_back(assembleFromKit(planet.seed(), *kitDef, candidate->anchor, 0));
        }
    }

    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    if (static_cast<int>(out.size()) > policy.maxSitesPerQuery) out.resize(static_cast<std::size_t>(policy.maxSitesPerQuery));
    return out;
}

std::vector<SiteDescriptor> SiteGenerator::generateStrategic(std::uint64_t systemSeed,
                                                             std::uint32_t bodyIndex,
                                                             int maxSites) const {
    std::vector<SiteDescriptor> out;
    if (maxSites <= 0) return out;
    static constexpr std::array<SiteFamily, 7> families = {
        SiteFamily::TradingPost, SiteFamily::ImperialStation, SiteFamily::UnswornHaven,
        SiteFamily::DerelictFreighter, SiteFamily::DataBuoy, SiteFamily::AsteroidRefinery,
        SiteFamily::CourtAnchorage,
    };
    const int count = 1 + static_cast<int>(mix64(systemSeed ^ bodyIndex ^ 0x5354524154454749ULL) % static_cast<std::uint64_t>(maxSites));
    for (int i = 0; i < count; ++i) {
        const std::uint64_t h = mix64(systemSeed ^ (static_cast<std::uint64_t>(bodyIndex) << 32U) ^ static_cast<std::uint64_t>(i));
        const SiteFamily family = families[h % families.size()];
        const auto* kitDef = catalog_.find(family);
        if (kitDef) out.push_back(assembleFromKit(systemSeed, *kitDef, std::nullopt, static_cast<std::uint32_t>(i + 1)));
    }
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    return out;
}

SiteIndexMergeResult SiteIndex::merge(const SiteDescriptor& descriptor) {
    if (descriptor.stableId == 0) return SiteIndexMergeResult::StableIdConflict;
    auto [it, inserted] = sites_.try_emplace(descriptor.stableId, descriptor);
    if (inserted) return SiteIndexMergeResult::Inserted;
    return it->second == descriptor ? SiteIndexMergeResult::Duplicate : SiteIndexMergeResult::StableIdConflict;
}

std::size_t SiteIndex::merge(const std::vector<SiteDescriptor>& descriptors, std::size_t* conflicts) {
    std::vector<const SiteDescriptor*> ordered;
    ordered.reserve(descriptors.size());
    for (const auto& d : descriptors) ordered.push_back(&d);
    std::sort(ordered.begin(), ordered.end(), [](const auto* a, const auto* b) { return a->stableId < b->stableId; });
    std::size_t inserted = 0;
    std::size_t conflictCount = 0;
    for (const auto* d : ordered) {
        switch (merge(*d)) {
            case SiteIndexMergeResult::Inserted: ++inserted; break;
            case SiteIndexMergeResult::Duplicate: break;
            case SiteIndexMergeResult::StableIdConflict: ++conflictCount; break;
        }
    }
    if (conflicts) *conflicts = conflictCount;
    return inserted;
}

const SiteDescriptor* SiteIndex::find(std::uint64_t stableId) const {
    const auto it = sites_.find(stableId);
    return it == sites_.end() ? nullptr : &it->second;
}

std::vector<SiteDescriptor> SiteIndex::all() const {
    std::vector<SiteDescriptor> out;
    out.reserve(sites_.size());
    for (const auto& [id, descriptor] : sites_) {
        (void)id;
        out.push_back(descriptor);
    }
    return out;
}

std::vector<SiteDescriptor> SiteIndex::resolved(const SiteStateStore& state) const {
    std::vector<SiteDescriptor> out;
    out.reserve(sites_.size());
    for (const auto& [id, descriptor] : sites_) {
        (void)id;
        out.push_back(state.apply(descriptor));
    }
    return out;
}

std::vector<SiteDescriptor> SiteIndex::querySurface(CubeFace face, int uBegin, int vBegin, int uEnd, int vEnd) const {
    std::vector<SiteDescriptor> out;
    if (uEnd <= uBegin || vEnd <= vBegin) return out;
    for (const auto& [id, descriptor] : sites_) {
        (void)id;
        if (!descriptor.anchor || descriptor.anchor->face != face) continue;
        const auto& a = *descriptor.anchor;
        if (a.u >= uBegin && a.u < uEnd && a.v >= vBegin && a.v < vEnd) out.push_back(descriptor);
    }
    return out;
}

std::uint64_t SiteIndex::fingerprint() const {
    std::uint64_t h = mix64(0x534954455F494E44ULL ^ static_cast<std::uint64_t>(sites_.size()));
    for (const auto& [id, d] : sites_) {
        h = mix64(h ^ id ^ (static_cast<std::uint64_t>(d.family) << 8U) ^ (static_cast<std::uint64_t>(d.domain) << 16U));
        h = mix64(h ^ (static_cast<std::uint64_t>(d.owner) << 24U) ^ (static_cast<std::uint64_t>(d.architecture) << 32U));
        if (d.anchor) h = hashAddress(h, *d.anchor, 0x494E44585F414444ULL);
        h = mix64(h ^ d.lootSeed ^ d.encounterSeed ^ static_cast<std::uint64_t>(d.modules.size()));
        for (const auto& m : d.modules)
            h = mix64(h ^ m.stableObjectId ^ (static_cast<std::uint64_t>(m.kind) << 9U) ^ (static_cast<std::uint64_t>(m.variant) << 17U));
    }
    return h;
}

const SitePersistentState* SiteStateStore::find(std::uint64_t siteId) const {
    const auto it = states_.find(siteId);
    return it == states_.end() ? nullptr : &it->second;
}

SitePersistentState& SiteStateStore::touch(const SiteDescriptor& baseline) {
    auto [it, inserted] = states_.try_emplace(baseline.stableId);
    if (inserted) {
        it->second.stableId = baseline.stableId;
        it->second.lifecycle = baseline.lifecycle;
        it->second.owner = baseline.owner;
        it->second.historicalOwnerId = baseline.historicalOwnerId;
    }
    return it->second;
}

SiteDescriptor SiteStateStore::apply(const SiteDescriptor& baseline) const {
    SiteDescriptor out = baseline;
    const auto* state = find(baseline.stableId);
    if (!state) return out;
    out.lifecycle = state->lifecycle;
    out.owner = state->owner;
    out.damage = satAdd(out.damage, state->addedDamage);
    out.contamination = std::max(out.contamination, state->contamination);
    if (state->historicalOwnerId != 0) out.historicalOwnerId = state->historicalOwnerId;
    if (state->populationOverride >= 0) out.settlement.baselinePopulation = state->populationOverride;
    for (auto& module : out.modules) {
        const auto it = state->objectStates.find(module.stableObjectId);
        if (it == state->objectStates.end()) continue;
        if (it->second == SiteObjectState::Destroyed || it->second == SiteObjectState::Tombstoned) module.damage = 100;
        else if (it->second == SiteObjectState::Disabled) module.damage = std::max<std::uint8_t>(module.damage, 75);
    }
    return out;
}

bool SiteStateStore::transitionAllowed(SiteLifecycleState from, SiteLifecycleState to) {
    if (from == to) return true;
    switch (from) {
        case SiteLifecycleState::GeneratedUnknown:
            return to == SiteLifecycleState::ActiveStrategic || to == SiteLifecycleState::ActiveDetailed ||
                   to == SiteLifecycleState::Abandoned || to == SiteLifecycleState::RuinedContaminated;
        case SiteLifecycleState::ActiveStrategic:
            return to == SiteLifecycleState::ActiveDetailed || to == SiteLifecycleState::BesiegedDisrupted ||
                   to == SiteLifecycleState::Abandoned || to == SiteLifecycleState::OccupiedReused;
        case SiteLifecycleState::ActiveDetailed:
            return to == SiteLifecycleState::ActiveStrategic || to == SiteLifecycleState::BesiegedDisrupted ||
                   to == SiteLifecycleState::Abandoned || to == SiteLifecycleState::RuinedContaminated;
        case SiteLifecycleState::BesiegedDisrupted:
            return to == SiteLifecycleState::ActiveStrategic || to == SiteLifecycleState::ActiveDetailed ||
                   to == SiteLifecycleState::Abandoned || to == SiteLifecycleState::RuinedContaminated ||
                   to == SiteLifecycleState::OccupiedReused;
        case SiteLifecycleState::Abandoned:
            return to == SiteLifecycleState::RuinedContaminated || to == SiteLifecycleState::OccupiedReused ||
                   to == SiteLifecycleState::Reclaimed;
        case SiteLifecycleState::RuinedContaminated:
            return to == SiteLifecycleState::OccupiedReused || to == SiteLifecycleState::Reclaimed;
        case SiteLifecycleState::OccupiedReused:
            return to == SiteLifecycleState::BesiegedDisrupted || to == SiteLifecycleState::Abandoned ||
                   to == SiteLifecycleState::RuinedContaminated || to == SiteLifecycleState::Reclaimed;
        case SiteLifecycleState::Reclaimed:
            return to == SiteLifecycleState::ActiveStrategic || to == SiteLifecycleState::ActiveDetailed ||
                   to == SiteLifecycleState::BesiegedDisrupted || to == SiteLifecycleState::Abandoned;
    }
    return false;
}

bool SiteStateStore::transition(const SiteDescriptor& baseline, SiteLifecycleState next, SiteFaction actor) {
    auto& s = touch(baseline);
    if (!transitionAllowed(s.lifecycle, next)) return false;
    const auto from = s.lifecycle;
    if (next == SiteLifecycleState::OccupiedReused || next == SiteLifecycleState::Reclaimed) {
        if (s.owner != SiteFaction::None) s.historicalOwnerId = mix64(baseline.stableId ^ static_cast<std::uint64_t>(s.owner) ^ kHistoryLabel);
        if (actor != SiteFaction::None) s.owner = actor;
    }
    if (next == SiteLifecycleState::Abandoned || next == SiteLifecycleState::RuinedContaminated) s.populationOverride = 0;
    s.lifecycle = next;
    if (next == SiteLifecycleState::Reclaimed && s.populationOverride < 0) s.populationOverride = 0;

    SiteHistoryEvent e{};
    e.siteId = baseline.stableId;
    e.from = from;
    e.to = next;
    e.actor = actor;
    e.sequence = nextHistorySequence_++;
    e.eventId = mix64(baseline.stableId ^ kHistoryLabel ^ e.sequence ^ static_cast<std::uint64_t>(next));
    history_.push_back(e);
    return true;
}

bool SiteStateStore::setObjectState(const SiteDescriptor& baseline, std::uint64_t objectId, SiteObjectState state) {
    if (std::none_of(baseline.modules.begin(), baseline.modules.end(), [objectId](const auto& m) { return m.stableObjectId == objectId; }))
        return false;
    touch(baseline).objectStates[objectId] = state;
    return true;
}

bool SiteStateStore::setPopulation(const SiteDescriptor& baseline, int population) {
    if (population < 0 || population > 1000000) return false;
    touch(baseline).populationOverride = population;
    return true;
}

bool SiteStateStore::setContamination(const SiteDescriptor& baseline, std::uint8_t contamination) {
    touch(baseline).contamination = static_cast<std::uint8_t>(clamp100(contamination));
    return true;
}

void SiteStateStore::markDiscovered(const SiteDescriptor& baseline) { touch(baseline).discovered = true; }

std::string SiteStateStore::serialize() const {
    std::ostringstream out;
    out << "ELYSIUM_SITE_STATE 1\n";
    out << "next " << nextHistorySequence_ << '\n';
    for (const auto& [id, s] : states_) {
        out << "site " << id << ' ' << static_cast<int>(s.lifecycle) << ' ' << static_cast<int>(s.owner) << ' '
            << s.populationOverride << ' ' << static_cast<int>(s.addedDamage) << ' ' << static_cast<int>(s.contamination) << ' '
            << (s.discovered ? 1 : 0) << ' ' << s.historicalOwnerId << ' ' << s.objectStates.size() << ' ' << stateFingerprint(s) << '\n';
        for (const auto& [objectId, state] : s.objectStates)
            out << "object " << id << ' ' << objectId << ' ' << static_cast<int>(state) << '\n';
    }
    for (const auto& e : history_)
        out << "event " << e.eventId << ' ' << e.siteId << ' ' << static_cast<int>(e.from) << ' ' << static_cast<int>(e.to) << ' '
            << static_cast<int>(e.actor) << ' ' << e.sequence << '\n';
    return out.str();
}

bool SiteStateStore::restore(std::string_view text, std::string* error) {
    std::istringstream in{std::string(text)};
    std::string magic; int version = 0;
    if (!(in >> magic >> version) || magic != "ELYSIUM_SITE_STATE" || version != 1) {
        if (error) *error = "unsupported site state header";
        return false;
    }
    std::map<std::uint64_t, SitePersistentState> parsedStates;
    std::vector<SiteHistoryEvent> parsedHistory;
    std::map<std::uint64_t, std::size_t> expectedObjects;
    std::map<std::uint64_t, std::uint64_t> expectedFingerprint;
    std::uint64_t next = 1;
    std::string tag;
    while (in >> tag) {
        if (tag == "next") {
            if (!(in >> next) || next == 0) { if (error) *error = "invalid site history sequence"; return false; }
        } else if (tag == "site") {
            SitePersistentState s{}; int lifecycle, owner, damage, contamination, discovered; std::size_t objectCount; std::uint64_t fp;
            if (!(in >> s.stableId >> lifecycle >> owner >> s.populationOverride >> damage >> contamination >> discovered >> s.historicalOwnerId >> objectCount >> fp) ||
                s.stableId == 0 || lifecycle < 0 || lifecycle > static_cast<int>(SiteLifecycleState::Reclaimed) ||
                owner < 0 || owner > static_cast<int>(SiteFaction::Hostile) || damage < 0 || damage > 100 || contamination < 0 || contamination > 100 ||
                (discovered != 0 && discovered != 1) || s.populationOverride < -1) {
                if (error) *error = "invalid site state record";
                return false;
            }
            s.lifecycle = static_cast<SiteLifecycleState>(lifecycle);
            s.owner = static_cast<SiteFaction>(owner);
            s.addedDamage = static_cast<std::uint8_t>(damage);
            s.contamination = static_cast<std::uint8_t>(contamination);
            s.discovered = discovered != 0;
            if (!parsedStates.emplace(s.stableId, s).second) { if (error) *error = "duplicate site state"; return false; }
            expectedObjects[s.stableId] = objectCount;
            expectedFingerprint[s.stableId] = fp;
        } else if (tag == "object") {
            std::uint64_t siteId, objectId; int state;
            if (!(in >> siteId >> objectId >> state) || objectId == 0 || state < 0 || state > static_cast<int>(SiteObjectState::Tombstoned)) {
                if (error) *error = "invalid site object state";
                return false;
            }
            auto it = parsedStates.find(siteId);
            if (it == parsedStates.end() || !it->second.objectStates.emplace(objectId, static_cast<SiteObjectState>(state)).second) {
                if (error) *error = "site object references missing/duplicate site";
                return false;
            }
        } else if (tag == "event") {
            SiteHistoryEvent e{}; int from, to, actor;
            if (!(in >> e.eventId >> e.siteId >> from >> to >> actor >> e.sequence) || e.eventId == 0 || e.siteId == 0 || e.sequence == 0 ||
                from < 0 || from > static_cast<int>(SiteLifecycleState::Reclaimed) || to < 0 || to > static_cast<int>(SiteLifecycleState::Reclaimed) ||
                actor < 0 || actor > static_cast<int>(SiteFaction::Hostile)) {
                if (error) *error = "invalid site history event";
                return false;
            }
            e.from = static_cast<SiteLifecycleState>(from); e.to = static_cast<SiteLifecycleState>(to); e.actor = static_cast<SiteFaction>(actor);
            parsedHistory.push_back(e);
        } else {
            if (error) *error = "unknown site state tag: " + tag;
            return false;
        }
    }
    for (const auto& [id, count] : expectedObjects) {
        const auto it = parsedStates.find(id);
        if (it == parsedStates.end() || it->second.objectStates.size() != count || stateFingerprint(it->second) != expectedFingerprint[id]) {
            if (error) *error = "site state object count/fingerprint mismatch";
            return false;
        }
    }
    std::sort(parsedHistory.begin(), parsedHistory.end(), [](const auto& a, const auto& b) { return a.sequence < b.sequence; });
    if (std::adjacent_find(parsedHistory.begin(), parsedHistory.end(), [](const auto& a, const auto& b){ return a.sequence == b.sequence || a.eventId == b.eventId; }) != parsedHistory.end()) {
        if (error) *error = "duplicate site history identity";
        return false;
    }
    std::map<std::uint64_t, SiteLifecycleState> chainState;
    std::uint64_t maxSequence = 0;
    for (const auto& e : parsedHistory) {
        const auto siteIt = parsedStates.find(e.siteId);
        if (siteIt == parsedStates.end()) {
            if (error) *error = "site history references missing site";
            return false;
        }
        const auto expectedEventId = mix64(e.siteId ^ kHistoryLabel ^ e.sequence ^ static_cast<std::uint64_t>(e.to));
        if (e.eventId != expectedEventId || !transitionAllowed(e.from, e.to)) {
            if (error) *error = "site history event integrity failure";
            return false;
        }
        const auto chainIt = chainState.find(e.siteId);
        if (chainIt != chainState.end() && chainIt->second != e.from) {
            if (error) *error = "site history lifecycle chain mismatch";
            return false;
        }
        chainState[e.siteId] = e.to;
        maxSequence = std::max(maxSequence, e.sequence);
    }
    for (const auto& [siteId, last] : chainState) {
        const auto it = parsedStates.find(siteId);
        if (it == parsedStates.end() || it->second.lifecycle != last) {
            if (error) *error = "site history final lifecycle mismatch";
            return false;
        }
    }
    if (next <= maxSequence) {
        if (error) *error = "site history next sequence would collide";
        return false;
    }
    states_ = std::move(parsedStates);
    history_ = std::move(parsedHistory);
    nextHistorySequence_ = next;
    return true;
}

const char* siteFamilyName(SiteFamily family) {
    static constexpr std::array<const char*, 30> names = {
        "Supply Cache", "Crashed Ship", "Survey Outpost", "Unsworn Camp", "Mining Rig", "Ruin", "Monolith", "Rift Anomaly",
        "Imperial Waystation", "Abandoned Farm", "Weather Station", "Subsurface Vault", "Buried Reactor", "Research Habitat",
        "Planetary Relay", "Smuggler Den", "Ancient Observatory", "Geothermal Plant", "Flooded Complex", "Orbital Debris Fall",
        "Trading Post", "Imperial Station", "Unsworn Haven", "Derelict Freighter", "Data Buoy", "Asteroid Refinery", "Silent Colony",
        "Siege Ruin", "Anomaly Labyrinth", "Court Anchorage"
    };
    const auto i = static_cast<std::size_t>(family);
    return i < names.size() ? names[i] : "Unknown Site";
}

const char* siteLifecycleName(SiteLifecycleState state) {
    static constexpr std::array<const char*, 8> names = {
        "generated/unknown", "active strategic", "active detailed", "besieged/disrupted",
        "abandoned", "ruined/contaminated", "occupied/reused", "reclaimed"
    };
    const auto i = static_cast<std::size_t>(state);
    return i < names.size() ? names[i] : "unknown";
}

} // namespace elysium
