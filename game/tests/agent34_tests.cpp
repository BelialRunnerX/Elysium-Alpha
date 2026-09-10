// Intended function: imported tests implementation for agent34_tests; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/Derelict.hpp"
#include "world/PoiSites.hpp"
#include "world/RiftExpedition.hpp"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace elysium;

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

void testAuthoredSiteCatalogAndSettlementProfiles() {
    SiteCatalog catalog;
    require(catalog.all().size() == 30, "Part 18 authored POI family catalogue is incomplete");
    for (int raw = static_cast<int>(SiteFamily::SupplyCache); raw <= static_cast<int>(SiteFamily::CourtAnchorage); ++raw) {
        const auto family = static_cast<SiteFamily>(raw);
        const auto* def = catalog.find(family);
        require(def != nullptr && !def->modules.empty(), std::string("missing semantic site kit: ") + siteFamilyName(family));
        require(std::any_of(def->modules.begin(), def->modules.end(), [](const auto& m){ return m.required; }),
                std::string("site kit has no required semantic module: ") + siteFamilyName(family));
    }

    const auto camp = settlementProfile(SettlementScale::Camp, SiteFaction::Unsworn, 1);
    require(camp.minPopulation == 3 && camp.maxPopulation == 8 && camp.baselinePopulation >= 3 && camp.baselinePopulation <= 8,
            "Camp population contract drifted from 3-8");
    require(camp.has(SettlementServiceTrade) && camp.has(SettlementServiceContracts), "Camp lost trade/contract services");
    const auto hamlet = settlementProfile(SettlementScale::Hamlet, SiteFaction::Frontier, 2);
    require(hamlet.minPopulation == 8 && hamlet.maxPopulation == 25 && hamlet.has(SettlementServiceFood) && hamlet.has(SettlementServiceRepair),
            "Hamlet population/service contract drifted");
    const auto town = settlementProfile(SettlementScale::FrontierTown, SiteFaction::Frontier, 3);
    require(town.minPopulation == 25 && town.maxPopulation == 80 && town.has(SettlementServiceMarket) && town.has(SettlementServiceShipPad),
            "Frontier town population/service contract drifted");
}

void testDeterministicSitePlacementAndPersistentObjects() {
    constexpr std::uint64_t seed = 0xA3417EULL;
    PlanetSurface planet(seed, PlanetClass::Temperate);
    SiteGenerator generator;
    SiteSpacingPolicy policy{};
    policy.placementCellColumns = 12;
    policy.minSpacingColumns = 8;
    policy.maxSitesPerQuery = 32;
    policy.candidateChance = 255;
    const SiteSurfaceQuery query{CubeFace::PositiveZ, 0, 0, PlanetSurface::FaceResolution, PlanetSurface::FaceResolution};

    const auto a = generator.generateSurface(planet, query, policy);
    const auto b = generator.generateSurface(planet, query, policy);
    require(!a.empty(), "site generator produced no sites for deterministic high-density fixture");
    require(a == b, "same seed/biome/query produced different site descriptors");
    for (std::size_t i = 0; i < a.size(); ++i) {
        require(a[i].stableId != 0 && a[i].anchor.has_value(), "surface site lacks stable identity/anchor");
        for (std::size_t j = i + 1; j < a.size(); ++j) {
            if (a[i].anchor->face != a[j].anchor->face) continue;
            const int du = a[i].anchor->u - a[j].anchor->u;
            const int dv = a[i].anchor->v - a[j].anchor->v;
            require(du * du + dv * dv >= policy.minSpacingColumns * policy.minSpacingColumns,
                    "deterministic POI spacing contract violated");
        }
    }

    // Biome/class is an explicit generation channel: same physical seed can
    // legitimately produce a different site distribution on a different class.
    PlanetSurface barren(seed, PlanetClass::Barren);
    const auto barrenSites = generator.generateSurface(barren, query, policy);
    require(a != barrenSites, "site placement ignored planet class/biome channel");

    // Strategic generation covers orbit/space families and remains deterministic.
    const auto strategicA = generator.generateStrategic(0x53595354454DULL, 7, 7);
    const auto strategicB = generator.generateStrategic(0x53595354454DULL, 7, 7);
    require(!strategicA.empty() && strategicA == strategicB, "strategic orbit/space site generation is not deterministic");
    require(std::all_of(strategicA.begin(), strategicA.end(), [](const auto& d){ return d.domain == SiteDomain::Orbit || d.domain == SiteDomain::Space; }),
            "strategic generator emitted a non-orbit/space site");

    // Stable object/tombstone state overlays a regenerated baseline rather than
    // becoming a second site generator.
    const SiteDescriptor baseline = generator.assemble(seed, SiteFamily::SiegeRuin, a.front().anchor);
    require(baseline.stableId != 0 && !baseline.modules.empty(), "authored ruin assembly failed");
    SiteStateStore store;
    require(store.transition(baseline, SiteLifecycleState::Abandoned), "generated -> abandoned lifecycle transition failed");
    require(store.transition(baseline, SiteLifecycleState::RuinedContaminated), "abandoned -> ruined lifecycle transition failed");
    require(store.setObjectState(baseline, baseline.modules.front().stableObjectId, SiteObjectState::Tombstoned), "site tombstone rejected baseline object");
    require(store.setContamination(baseline, 81), "site contamination mutation failed");
    store.markDiscovered(baseline);
    require(store.transition(baseline, SiteLifecycleState::Reclaimed, SiteFaction::Player), "ruined -> reclaimed lifecycle transition failed");

    const std::string saved = store.serialize();
    SiteStateStore restored;
    std::string error;
    require(restored.restore(saved, &error), "site state persistence round trip failed: " + error);
    require(restored.serialize() == saved, "site state codec is not canonical after round trip");

    const SiteDescriptor regenerated = generator.assemble(seed, SiteFamily::SiegeRuin, a.front().anchor);
    require(regenerated.stableId == baseline.stableId && regenerated.modules == baseline.modules,
            "regeneration changed stable site/object identities");
    const SiteDescriptor reclaimed = restored.apply(regenerated);
    require(reclaimed.lifecycle == SiteLifecycleState::Reclaimed && reclaimed.owner == SiteFaction::Player && reclaimed.contamination >= 81,
            "reclaimed ruin did not restore lifecycle/owner/contamination");
    const auto objectIt = std::find_if(reclaimed.modules.begin(), reclaimed.modules.end(), [&](const auto& m){ return m.stableObjectId == baseline.modules.front().stableObjectId; });
    require(objectIt != reclaimed.modules.end() && objectIt->damage == 100, "destroyed/tombstoned site object regenerated pristine");
    require(restored.history().size() == 3, "site lifecycle did not append stable history transitions");

    // Reclaim overlays metadata only. A player voxel scar remains in the sparse
    // PlanetSurface journal and is never healed by site regeneration/state apply.
    const auto scar = *a.front().anchor;
    const BlockType original = planet.get(scar);
    const BlockType scarType = original == BlockType::Air ? BlockType::Stone : BlockType::Air;
    planet.set(scar, scarType, scarType != BlockType::Air);
    const auto editsBefore = planet.macroEditCount();
    (void)restored.apply(generator.assemble(seed, SiteFamily::SiegeRuin, a.front().anchor));
    require(planet.get(scar) == scarType && planet.macroEditCount() == editsBefore,
            "site reclaim/regeneration rewrote a player-authored voxel scar");
}

void testSparseSiteIndexAndPersistenceIntegrity() {
    constexpr std::uint64_t seed = 0x34A11CEULL;
    PlanetSurface planet(seed, PlanetClass::Temperate);
    SiteGenerator generator;
    SiteSpacingPolicy policy{};
    policy.placementCellColumns = 12;
    policy.minSpacingColumns = 8;
    policy.maxSitesPerQuery = 128;
    policy.candidateChance = 255;

    const int half = PlanetSurface::FaceResolution / 2;
    const std::vector<SiteSurfaceQuery> shards = {
        {CubeFace::PositiveZ, 0, 0, half, half},
        {CubeFace::PositiveZ, half, 0, PlanetSurface::FaceResolution, half},
        {CubeFace::PositiveZ, 0, half, half, PlanetSurface::FaceResolution},
        {CubeFace::PositiveZ, half, half, PlanetSurface::FaceResolution, PlanetSurface::FaceResolution},
    };

    SiteIndex streamed;
    for (const auto& q : shards) {
        std::size_t conflicts = 0;
        (void)streamed.merge(generator.generateSurface(planet, q, policy), &conflicts);
        require(conflicts == 0, "streaming site shards produced a StableId descriptor conflict");
    }
    require(!streamed.empty(), "sparse site index received no generated sites");

    SiteIndex monolithic;
    const SiteSurfaceQuery full{CubeFace::PositiveZ, 0, 0, PlanetSurface::FaceResolution, PlanetSurface::FaceResolution};
    std::size_t conflicts = 0;
    monolithic.merge(generator.generateSurface(planet, full, policy), &conflicts);
    require(conflicts == 0 && monolithic.fingerprint() == streamed.fingerprint() && monolithic.all() == streamed.all(),
            "site index publication depends on streaming query partition/order");

    const auto baseline = streamed.all().front();
    require(streamed.merge(baseline) == SiteIndexMergeResult::Duplicate, "identical site re-publication was not deduplicated");
    auto conflicting = baseline;
    conflicting.owner = conflicting.owner == SiteFaction::Imperial ? SiteFaction::Unsworn : SiteFaction::Imperial;
    require(streamed.merge(conflicting) == SiteIndexMergeResult::StableIdConflict,
            "same StableId with different baseline descriptor was silently accepted");
    require(streamed.find(baseline.stableId) != nullptr && streamed.find(baseline.stableId)->owner == baseline.owner,
            "StableId conflict replaced the accepted baseline");

    const auto local = streamed.querySurface(CubeFace::PositiveZ, 0, 0, half, half);
    require(std::all_of(local.begin(), local.end(), [half](const auto& d) {
        return d.anchor && d.anchor->face == CubeFace::PositiveZ && d.anchor->u < half && d.anchor->v < half;
    }), "site index surface query leaked descriptors outside its bounded region");

    SiteStateStore store;
    require(store.transition(baseline, SiteLifecycleState::Abandoned), "site index fixture could not abandon site");
    const auto resolved = streamed.resolved(store);
    const auto resolvedIt = std::find_if(resolved.begin(), resolved.end(), [&](const auto& d) { return d.stableId == baseline.stableId; });
    require(resolvedIt != resolved.end() && resolvedIt->lifecycle == SiteLifecycleState::Abandoned,
            "site index did not overlay persistent lifecycle state");

    const std::string saved = store.serialize();
    const auto nextPos = saved.find("next ");
    require(nextPos != std::string::npos, "site state fixture missing next sequence");
    const auto nextEnd = saved.find('\n', nextPos);
    std::string badNext = saved;
    badNext.replace(nextPos, nextEnd - nextPos, "next 1");
    SiteStateStore rejected;
    std::string error;
    require(!rejected.restore(badNext, &error), "site state accepted a history sequence collision");

    const auto eventPos = saved.find("event ");
    require(eventPos != std::string::npos, "site state fixture missing history event");
    const auto idBegin = eventPos + 6;
    const auto idEnd = saved.find(' ', idBegin);
    const auto eventId = std::stoull(saved.substr(idBegin, idEnd - idBegin));
    std::string badEvent = saved;
    badEvent.replace(idBegin, idEnd - idBegin, std::to_string(eventId + 1));
    require(!rejected.restore(badEvent, &error), "site state accepted a corrupted history event identity");
}

void testDerelictOperationalStateAndPersistence() {
    SiteGenerator generator;
    const auto descriptor = generator.assemble(0xD3E311C7ULL, SiteFamily::DerelictFreighter, std::nullopt, 1);
    DerelictState derelict = DerelictState::generate(descriptor, 2);
    require(derelict.sections().size() == 2 && !derelict.links().empty(), "derelict did not build bounded section graph");
    require(derelict.setBulkhead(0, 1, true), "derelict test could not open connecting bulkhead");

    auto* target = derelict.section(1);
    require(target != nullptr, "derelict target section missing");
    target->pressure = 0.0f;
    target->fire = 0.0f;
    target->contamination = 0.0f;
    target->gravity = 1.0f;
    target->security = DerelictSecurityState::Offline;

    DerelictTraversalPolicy ordinary{};
    auto blocked = derelict.reachable(0, ordinary);
    require(blocked.size() == 1 && blocked.front() == 0, "vacuum did not meaningfully block derelict traversal");
    DerelictTraversalPolicy suited{};
    suited.vacuumProtection = true;
    require(derelict.reachable(0, suited).size() == 2, "vacuum protection did not restore derelict traversal");

    // Local power + emergency sealing can turn an unsafe section into a route.
    require(derelict.setPowered(1, true) && derelict.emergencySeal(1, true), "derelict power/seal operation failed");
    derelict.update(10.0f);
    require(derelict.section(1)->pressure >= 0.35f && derelict.reachable(0, ordinary).size() == 2,
            "powered emergency seal did not restore traversable pressure");

    derelict.section(1)->fire = 1.0f;
    require(derelict.reachable(0, ordinary).size() == 1, "derelict fire did not alter traversal");
    DerelictTraversalPolicy fireSafe{};
    fireSafe.fireProtection = true;
    require(derelict.reachable(0, fireSafe).size() == 2, "fire protection did not permit traversal");
    const int taken = derelict.salvage(1, 7);
    require(taken >= 0 && derelict.section(1)->salvagedMass >= taken, "derelict salvage accounting failed");

    const std::string saved = derelict.serialize();
    DerelictState restored;
    std::string error;
    require(restored.restore(saved, &error), "derelict operational state did not restore: " + error);
    require(restored.serialize() == saved, "derelict codec changed state after round trip");
    const auto before = derelict.inspect();
    const auto after = restored.inspect();
    require(before.siteId == after.siteId && before.sections == after.sections && before.poweredSections == after.poweredSections &&
            before.burningSections == after.burningSections && before.salvageMassRemaining == after.salvageMassRemaining,
            "derelict persistence lost operational inspection state");
}

void testRiftTopologyDeterminismAndContract() {
    for (std::uint64_t seed = 0; seed < 400; ++seed) {
        const auto a = RiftTopology::generate(seed);
        const auto b = RiftTopology::generate(seed);
        require(a.rooms() == b.rooms() && a.fingerprint() == b.fingerprint(), "same Rift seed produced different topology");
        const auto validation = a.validate();
        require(validation.valid, "Rift topology failed contract at seed " + std::to_string(seed) + ": " + validation.reason);
        int loot = 0, boss = 0;
        for (const auto& room : a.rooms()) {
            if (room.kind == RiftRoomKind::Loot) {
                ++loot;
                require(std::popcount(static_cast<unsigned>(room.doors)) == 1, "Rift loot room is not a dead end");
            }
            if (room.kind == RiftRoomKind::Boss) ++boss;
        }
        require(loot == RiftTopology::LootRoomCount && boss == 1, "Rift special-room assignment drifted");
    }
}

void testRiftExpeditionState() {
    RiftRunState run = RiftRunState::begin(0xA110A1ULL, 0xE7A6D1ULL, 73, PlanetClass::Scorched);
    require(run.active() && run.depth() == 1 && run.topology().validate().valid, "Rift run did not start on valid floor 1");
    const auto s1 = run.scaling();
    require(s1.depth == 1 && s1.openerLevel == 73, "Rift scaling did not capture portal opener level");
    require(run.addLoot(100, 8), "Rift loot/heat accumulation failed");
    const auto offer = run.boonOffer();
    require(offer[0] != offer[1] && offer[0] != offer[2] && offer[1] != offer[2], "Rift boon offer contains duplicates");
    require(run.acceptBoon(offer[0]), "Rift offered boon could not be accepted");
    require(!run.acceptBoon(offer[0]), "duplicate Rift boon was accepted");
    const auto floor1 = run.topology().fingerprint();
    require(run.descend(true) && run.depth() == 2, "boss descent did not advance Rift depth");
    require(run.scaling().depthDominantRank > s1.depthDominantRank, "Rift depth did not dominate scaling rank");
    require(run.topology().fingerprint() != floor1, "Rift descent reused the same floor topology seed");

    const auto exit = run.bankAndExit();
    require(exit.bankedGained == 100 && exit.persistentSuspicionHeat == 2 && !exit.ejected && exit.gearSurvives && exit.instanceCollapsed,
            "Rift bank/exit consequences drifted");
    require(run.bankedValue() == 100 && run.carriedValue() == 0 && run.depth() == 1 && run.boons().empty() && !run.active(),
            "Rift exit did not clear run-scoped state while preserving banked value");

    run.restart();
    require(run.active() && run.bankedValue() == 100 && run.depth() == 1, "Rift restart lost banked progression");
    require(run.addLoot(55, 13), "Rift second-run loot failed");
    const auto death = run.die();
    require(death.carriedLost == 55 && death.ejected && death.gearSurvives && death.instanceCollapsed && death.persistentSuspicionHeat == 0,
            "Rift death consequences drifted");
    require(run.bankedValue() == 100 && run.carriedValue() == 0 && !run.active(), "Rift death lost banked value or retained run loot");

    run.restart();
    require(run.addLoot(9, 3), "Rift persistence fixture could not add loot");
    const auto offer2 = run.boonOffer();
    require(run.acceptBoon(offer2[1]), "Rift persistence fixture could not accept boon");
    const std::string saved = run.serialize();
    RiftRunState restored;
    std::string error;
    require(restored.restore(saved, &error), "Rift run state failed persistence round trip: " + error);
    require(restored.serialize() == saved && restored.boons() == run.boons() && restored.topology().fingerprint() == run.topology().fingerprint(),
            "Rift run persistence changed deterministic state");
}

} // namespace

int main() {
    try {
        testAuthoredSiteCatalogAndSettlementProfiles();
        testDeterministicSitePlacementAndPersistentObjects();
        testSparseSiteIndexAndPersistenceIntegrity();
        testDerelictOperationalStateAndPersistence();
        testRiftTopologyDeterminismAndContract();
        testRiftExpeditionState();
        std::cout << "Elysium Agent 34 tests: PASS\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Elysium Agent 34 tests: FAIL: " << e.what() << '\n';
        return 1;
    }
}
