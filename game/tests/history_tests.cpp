// Intended function: imported tests implementation for history_tests; preserves the agent-authored subsystem contract for later integration/debugging.
#include "history/GalacticChronicle.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace elysium;

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

std::vector<HistoryOpportunity> fixtureOpportunities() {
    return {
        {9004, 401, 2, 71, 84},
        {9001, 17, 0, 82, 52},
        {9006, 777, 1, 44, 96},
        {9003, 256, 3, 64, 63},
        {9002, 89, 1, 90, 45},
        {9005, 615, 0, 55, 78}
    };
}

std::uint64_t buildHistoryDigest(std::size_t workers) {
    WorldHistoryGenerationConfig config{};
    config.galaxySeed = 0xE17A37C0FFEEULL;
    config.civilizationCount = 4;
    WorldHistoryGenerator generator(config);
    auto chronicle = generator.generateFoundation(fixtureOpportunities());
    if (workers == 0) {
        JobSystem jobs(SerialJobs);
        for (std::uint32_t epoch = 0; epoch < 12; ++epoch) generator.simulateEpoch(jobs, chronicle, epoch, 1000ULL * (epoch + 1ULL));
    } else {
        JobSystem jobs(workers);
        for (std::uint32_t epoch = 0; epoch < 12; ++epoch) generator.simulateEpoch(jobs, chronicle, epoch, 1000ULL * (epoch + 1ULL));
    }
    return chronicle.deterministicDigest();
}

void testHistoryGeneratorWorkerDeterminism() {
    const auto serial = buildHistoryDigest(0);
    require(serial != 0, "history digest is zero");
    for (const std::size_t workers : {1U, 2U, 4U, 8U})
        require(buildHistoryDigest(workers) == serial, "history output changed across worker counts");
}

void testSignificancePromotionPolicy() {
    HistoricalSignificancePolicy policy;
    HistoricalSignificanceInput anonymous{};
    require(!policy.shouldPromote(anonymous), "anonymous remote worker was promoted without a significance trigger");

    HistoricalSignificanceInput office{}; office.holdsOffice = true;
    require(!policy.shouldPromote(office), "single low-weight office trigger exceeded default significance threshold unexpectedly");
    office.discovery = true;
    require(policy.shouldPromote(office), "office + discovery did not promote a historical figure");

    HistoricalSignificanceInput artifact{}; artifact.ownsArtifact = true;
    require(policy.shouldPromote(artifact), "artifact ownership did not independently promote a historical figure");
}

void testChronicleCrossLinksPersistenceAndRetireReclaim() {
    GalacticChronicle chronicle(0xC0FFEE37ULL);
    std::string error;

    const OrganizationId civ = 0x1001;
    const OrganizationId guild = 0x1002;
    const SiteId site = 0x2001;
    const HistoryStableId maker = 0x3001;
    const HistoryStableId ownerA = 0x3002;
    const HistoryStableId ownerB = 0x3003;
    const ArtifactId artifactId = 0x4001;

    require(chronicle.registerCivilization({civ, "Emerald Reach", "Archive frontier", "Chartered council", 42, 3, 67, {site}, {guild}}, &error), error);
    require(chronicle.registerSite({site, "Nacre Bastion", 42, 1, civ, SiteStatus::Active, 733}, &error), error);
    require(chronicle.registerOrganization({guild, OrganizationKind::Guild, "Surveyor Compact", civ, site, "Cartography before conquest", 48, 72}, &error), error);
    require(chronicle.registerFigure({maker, "Ilex Mora", site, site, guild, 100, 14, true}, &error), error);
    require(chronicle.registerFigure({ownerA, "Tern Vale", site, site, guild, 120, 12, true}, &error), error);
    require(chronicle.registerFigure({ownerB, "Rhea Cind", site, site, guild, 140, 12, true}, &error), error);

    HistoricalEventDraft founded{}; founded.timestamp = 200; founded.type = HistoryEventType::SiteFounded; founded.location = {42, 1, site}; founded.organizations = {civ}; founded.importance = 80;
    const auto foundedId = chronicle.append(founded, &error); require(foundedId != 0, error);

    HistoricalEventDraft created{}; created.timestamp = 300; created.type = HistoryEventType::ArtifactCreated; created.location = {42, 1, site}; created.participants = {{maker, "maker"}, {ownerA, "first_owner"}}; created.artifacts = {artifactId}; created.organizations = {guild}; created.causeRefs = {foundedId}; created.payload = {{"material", "Aetherium"}, {"name", "The Quiet Index"}}; created.importance = 95;
    const auto createdId = chronicle.append(created, &error); require(createdId != 0, error);
    require(chronicle.registerArtifact({artifactId, "The Quiet Index", maker, ownerA, site, createdId}, &error), error);

    HistoricalEventDraft gift{}; gift.timestamp = 450; gift.type = HistoryEventType::ArtifactGifted; gift.location = {42, 1, site}; gift.participants = {{ownerA, "giver"}, {ownerB, "recipient"}}; gift.artifacts = {artifactId}; gift.causeRefs = {createdId}; gift.importance = 70;
    const auto giftId = chronicle.append(gift, &error); require(giftId != 0, error);
    require(chronicle.artifact(artifactId) && chronicle.artifact(artifactId)->currentOwnerStableId == ownerB, "artifact durable owner summary did not follow ownership event");

    const auto retiredId = chronicle.appendFortressRetired(600, site, ownerB, &error); require(retiredId != 0, error);
    require(chronicle.site(site)->status == SiteStatus::Retired, "retirement did not update durable site status");
    const auto reclaimedId = chronicle.appendFortressReclaimed(900, site, ownerB, &error); require(reclaimedId != 0, error);
    require(chronicle.site(site)->status == SiteStatus::Reclaimed, "reclamation did not update durable site status");

    HistoricalEventDraft intervention{}; intervention.timestamp = 950; intervention.type = HistoryEventType::DirectOperativeIntervention; intervention.location = {42, 1, site}; intervention.participants = {{ownerB, "operative"}}; intervention.causeRefs = {reclaimedId}; intervention.importance = 90;
    const auto interventionId = chronicle.append(intervention, &error); require(interventionId != 0, error);

    const auto chain = chronicle.eventsForArtifact(artifactId);
    require(chain.size() == 2 && chain.front().eventId == createdId && chain.back().eventId == giftId, "artifact ownership/provenance chain is not queryable in time order");
    const auto timeline = chronicle.eventsForSite(site);
    require(timeline.size() >= 6 && timeline.front().eventId == foundedId, "site timeline did not retain founding through retire/reclaim/player history");
    const auto causes = chronicle.traceCauses(interventionId);
    require(causes.size() == 2 && causes.front().eventId == reclaimedId && causes.back().eventId == interventionId, "causal history trace failed");

    const auto comparison = chronicle.compareEntities(ownerA, ownerB);
    require(!comparison.shared.empty(), "figure comparison did not expose shared artifact event");
    const auto search = chronicle.search("nacre");
    require(search.size() == 1 && search.front().kind == ChronicleSearchKind::Site && search.front().id == site, "Chronicle search did not find site by stable name");

    const auto beforeDigest = chronicle.deterministicDigest();
    const auto text = chronicle.serialize();
    require(text.find("entt::entity") == std::string::npos, "Chronicle persistence leaked ECS runtime identity text");
    GalacticChronicle restored;
    require(restored.restore(text, &error), "Chronicle restore failed: " + error);
    require(restored.deterministicDigest() == beforeDigest, "Chronicle save/load changed deterministic bytes");
    require(restored.eventsForArtifact(artifactId).size() == chain.size(), "artifact index failed to rebuild from authoritative events");
    require(restored.eventsForSite(site).size() == timeline.size(), "site index failed to rebuild from authoritative events");
    require(restored.event(interventionId) != nullptr, "immutable event ID did not survive save/load");

    // Derived indexes are never serialized as a second authority. Rebuilding
    // them from events must not change the canonical persistent form.
    const auto stableBytes = restored.serialize();
    restored.rebuildIndexes();
    require(restored.serialize() == stableBytes, "derived Chronicle index rebuild changed authoritative bytes");
}

void testChronicleValidationAndFiltering() {
    GalacticChronicle chronicle(12345);
    std::string error;
    require(chronicle.registerSite({700, "Archive Delta", 77, 4, 0, SiteStatus::Active, 12}, &error), error);
    require(!chronicle.registerSite({700, "Duplicate", 77, 4, 0, SiteStatus::Active, 1}, &error), "duplicate SiteId was accepted");

    HistoricalEventDraft bad{}; bad.timestamp = 1; bad.type = HistoryEventType::Battle; bad.causeRefs = {999999};
    require(chronicle.append(bad, &error) == 0, "event with unresolved cause was accepted");

    HistoricalEventDraft e1{}; e1.timestamp = 10; e1.type = HistoryEventType::Discovery; e1.location = {77, 4, 700}; e1.participants = {{101, "discoverer"}};
    const auto id1 = chronicle.append(e1, &error); require(id1 != 0, error);
    HistoricalEventDraft e2{}; e2.timestamp = 20; e2.type = HistoryEventType::Battle; e2.location = {77, 4, 700}; e2.participants = {{101, "defender"}, {102, "attacker"}}; e2.causeRefs = {id1};
    require(chronicle.append(e2, &error) != 0, error);

    ChronicleQuery q{}; q.systemIndex = 77; q.fromTimestamp = 15; q.toTimestamp = 25;
    const auto filtered = chronicle.query(q);
    require(filtered.size() == 1 && filtered.front().type == HistoryEventType::Battle, "Chronicle time/system map filter returned wrong event set");

    auto serialized = chronicle.serialize();
    const auto pos = serialized.find("end_event\n");
    require(pos != std::string::npos, "fixture could not find event terminator");
    // Corrupt the first event's immutable ID into zero; restore must reject it.
    const auto eventPos = serialized.find("event ");
    require(eventPos != std::string::npos, "fixture could not find event record");
    const auto idStart = eventPos + 6;
    const auto idEnd = serialized.find(' ', idStart);
    serialized.replace(idStart, idEnd - idStart, "0");
    GalacticChronicle invalid;
    require(!invalid.restore(serialized, &error), "Chronicle accepted a zero immutable event ID");
}

void testStrategicRelationsOwnershipLineageAndLoss() {
    GalacticChronicle chronicle(0x37A11CEULL);
    std::string error;

    const OrganizationId civA = 0xA001;
    const OrganizationId civB = 0xB001;
    const OrganizationId parentGuild = 0xA100;
    const OrganizationId childGuild = 0xA101;
    const SiteId siteA = 0x5100;
    const SiteId siteB = 0x5200;
    const ArtifactId artifact = 0x5300;
    const HistoryStableId owner = 0x5400;

    require(chronicle.registerCivilization({civA, "Aster Compact", "Archive", "Council", 10, 3, 50, {siteA}, {parentGuild}}, &error), error);
    require(chronicle.registerCivilization({civB, "Vara March", "Frontier", "Prefecture", 11, 4, 60, {siteB}, {}}, &error), error);
    require(chronicle.registerSite({siteA, "Pale Archive", 10, 0, civA, SiteStatus::Active, 500}, &error), error);
    require(chronicle.registerSite({siteB, "Green Bastion", 11, 0, civB, SiteStatus::Active, 420}, &error), error);
    require(chronicle.registerOrganization({parentGuild, OrganizationKind::Guild, "Cartographers of Pale Archive", civA, siteA, "Trace every route", 42, 63}, &error), error);

    HistoricalEventDraft war{};
    war.timestamp = 100;
    war.type = HistoryEventType::WarDeclared;
    war.organizations = {civA, civB};
    war.importance = 85;
    const auto warId = chronicle.append(war, &error); require(warId != 0, error);
    auto relation = chronicle.relationBetween(civA, civB);
    require(relation.stance == DiplomaticStance::War && relation.lastEventId == warId, "war did not become current strategic relation");

    HistoricalEventDraft treaty{};
    treaty.timestamp = 200;
    treaty.type = HistoryEventType::Treaty;
    treaty.organizations = {civB, civA};
    treaty.causeRefs = {warId};
    const auto treatyId = chronicle.append(treaty, &error); require(treatyId != 0, error);
    relation = chronicle.relationBetween(civA, civB);
    require(relation.stance == DiplomaticStance::Treaty && relation.lastEventId == treatyId, "later treaty did not supersede war relation");
    const auto civARelations = chronicle.relationsForOrganization(civA);
    require(civARelations.size() == 1 && civARelations.front().stance == DiplomaticStance::Treaty, "organization relation index/query was not stable");

    const auto occupied = chronicle.appendSiteOwnershipChange(300, HistoryEventType::Occupation, siteA, civB, {}, {treatyId}, &error);
    require(occupied != 0, error);
    require(chronicle.site(siteA)->ownerOrganizationId == civB, "occupation did not update durable site owner");
    require(std::find(chronicle.civilization(civA)->territorySites.begin(), chronicle.civilization(civA)->territorySites.end(), siteA) == chronicle.civilization(civA)->territorySites.end(), "occupied site remained in prior civilization territory");
    require(std::find(chronicle.civilization(civB)->territorySites.begin(), chronicle.civilization(civB)->territorySites.end(), siteA) != chronicle.civilization(civB)->territorySites.end(), "occupied site was not added to new civilization territory");

    const auto liberated = chronicle.appendSiteOwnershipChange(400, HistoryEventType::Liberation, siteA, civA, {}, {occupied}, &error);
    require(liberated != 0, error);
    const auto ownership = chronicle.siteOwnershipTimeline(siteA);
    require(ownership.size() == 2, "site ownership timeline should contain occupation and liberation transitions");
    require(ownership[0].previousOwner == civA && ownership[0].newOwner == civB, "occupation ownership transition was not reconstructed");
    require(ownership[1].previousOwner == civB && ownership[1].newOwner == civA, "liberation ownership transition was not reconstructed");

    OrganizationRecord child{childGuild, OrganizationKind::ResearchCircle, "Index Schism", civA, siteA, "Knowledge without filing", 31, 44};
    const auto schismId = chronicle.appendOrganizationSchism(450, parentGuild, child, siteA, &error);
    require(schismId != 0, error);
    const auto lineage = chronicle.organizationLineage(childGuild);
    require(lineage.size() == 2 && lineage[0] == parentGuild && lineage[1] == childGuild, "organization schism lineage was not queryable");

    require(chronicle.registerFigure({owner, "Ravel Quill", siteA, siteA, parentGuild, 1, 12, true}, &error), error);
    HistoricalEventDraft created{};
    created.timestamp = 500; created.type = HistoryEventType::ArtifactCreated; created.location = {10, 0, siteA};
    created.participants = {{owner, "first_owner"}}; created.artifacts = {artifact};
    const auto createdId = chronicle.append(created, &error); require(createdId != 0, error);
    require(chronicle.registerArtifact({artifact, "The Unfiled Map", owner, owner, siteA, createdId}, &error), error);
    HistoricalEventDraft lost{};
    lost.timestamp = 550; lost.type = HistoryEventType::ArtifactLost; lost.location = {10, 0, siteA}; lost.artifacts = {artifact}; lost.causeRefs = {createdId};
    require(chronicle.append(lost, &error) != 0, error);
    require(chronicle.artifact(artifact)->currentOwnerStableId == 0, "lost artifact retained an owner summary");
    const auto artifactOwners = chronicle.artifactOwnershipTimeline(artifact);
    require(artifactOwners.size() == 2 && artifactOwners.back().newOwner == 0, "artifact loss was not present in ownership timeline");

    const auto relationBeforeSave = chronicle.relationBetween(civA, civB);
    const auto bytes = chronicle.serialize();
    GalacticChronicle restored;
    require(restored.restore(bytes, &error), "strategic Chronicle restore failed: " + error);
    require(restored.relationBetween(civA, civB) == relationBeforeSave, "diplomatic relation did not survive save/reload from events");
    require(restored.site(siteA)->ownerOrganizationId == civA, "final site ownership did not survive save/reload");
    require(restored.organizationLineage(childGuild) == lineage, "organization lineage did not survive save/reload");
    require(restored.artifactOwnershipTimeline(artifact) == artifactOwners, "artifact ownership timeline changed after save/reload");

    require(chronicle.appendSiteOwnershipChange(600, HistoryEventType::Occupation, siteA, 0xDEADBEEF, {}, {}, &error) == 0,
            "unknown site owner organization was accepted");
}

} // namespace

int main() {
    try {
        testHistoryGeneratorWorkerDeterminism();
        testSignificancePromotionPolicy();
        testChronicleCrossLinksPersistenceAndRetireReclaim();
        testChronicleValidationAndFiltering();
        testStrategicRelationsOwnershipLineageAndLoss();
        std::cout << "Elysium history tests: PASS\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Elysium history tests: FAIL: " << e.what() << '\n';
        return 1;
    }
}
