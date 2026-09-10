// Intended function: imported tests implementation for citizen_mind_tests; preserves the agent-authored subsystem contract for later integration/debugging.
#include "ecs/CitizenMind.hpp"

#include <algorithm>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace elysium;

namespace {

bool near(float a, float b, float eps = 1e-5f) { return std::fabs(a - b) <= eps; }

CitizenMindState makeCitizen(CitizenStableId id) {
    CitizenMindState s{};
    s.stableId = id;
    s.personality.facets.fill(0.5f);
    s.values.importance.fill(0.5f);
    for (auto& n : s.needs.dimensions) {
        n.strength = 0.65f;
        n.satisfaction = 1.0f;
    }
    s.personality.facets[static_cast<std::size_t>(PersonalityFacet::Anxiety)] = 0.75f;
    s.personality.facets[static_cast<std::size_t>(PersonalityFacet::Discipline)] = 0.35f;
    s.personality.facets[static_cast<std::size_t>(PersonalityFacet::Sociability)] = 0.70f;
    s.values.importance[static_cast<std::size_t>(ValueDimension::Family)] = 0.90f;
    CitizenMindSystem::normalize(s);
    return s;
}

MindEvent lossEvent(std::uint64_t eventId, SimTimeMs time) {
    MindEvent e{};
    e.eventId = eventId;
    e.time = time;
    e.type = MindEventType::Loss;
    e.valence = -1.0f;
    e.intensity = 1.0f;
    e.chronicleImportance = 0.9f;
    e.thoughtKind = ThoughtKind::Grief;
    e.topic = "elysium:event/founder_death";
    e.reaction.value = ValueDimension::Family;
    e.reaction.valueWeight = 1.0f;
    return e;
}

void testNeedThoughtMemoryStressFocusAndDiagnostics() {
    CitizenMindState citizen = makeCitizen(1001);
    citizen.needs.dimensions[static_cast<std::size_t>(NeedType::Sleep)].strength = 1.0f;
    citizen.needs.dimensions[static_cast<std::size_t>(NeedType::Safety)].strength = 1.0f;

    auto sleep = CitizenMindSystem::updateNeed(citizen, NeedType::Sleep, 0.08f, 1'000, 11);
    assert(sleep.thoughtCreated);
    assert(!sleep.events.empty());
    assert(sleep.events.front().type == MindDomainEventType::NeedThresholdCrossed);
    assert(citizen.focus.value < 1.0f);

    auto safety = CitizenMindSystem::updateNeed(citizen, NeedType::Safety, 0.05f, 2'000, 12);
    assert(safety.thoughtCreated);

    const auto loss = CitizenMindSystem::applyEvent(citizen, lossEvent(13, 3'000));
    assert(loss.thoughtCreated);
    assert(loss.memoryFormed);
    assert(!citizen.memories.entries.empty());
    assert(citizen.memories.entries.back().traumatic);
    assert(citizen.stress.band == StressBand::Crisis || citizen.stress.band == StressBand::Distressed);

    if (citizen.stress.band != StressBand::Crisis) {
        MindEvent danger{};
        danger.eventId = 14;
        danger.time = 4'000;
        danger.type = MindEventType::Danger;
        danger.valence = -1.0f;
        danger.intensity = 1.0f;
        danger.chronicleImportance = 0.6f;
        danger.topic = "elysium:event/decompression";
        danger.reaction.facet = PersonalityFacet::Anxiety;
        danger.reaction.facetWeight = 1.0f;
        const auto r = CitizenMindSystem::applyEvent(citizen, danger);
        assert(r.thoughtCreated);
    }
    assert(citizen.stress.band == StressBand::Crisis);

    const WorkReadiness readiness = CitizenMindSystem::evaluateWorkReadiness(citizen);
    assert(!readiness.canWork);
    assert(readiness.blocker == MindWorkBlocker::Crisis);

    const MindDiagnostic diagnostic = CitizenMindSystem::inspect(citizen, 5'000);
    assert(diagnostic.stableId == citizen.stableId);
    assert(diagnostic.band == StressBand::Crisis);
    assert(!diagnostic.reasons.empty());
    assert(std::any_of(diagnostic.reasons.begin(), diagnostic.reasons.end(), [](const auto& reason) {
        return reason.kind == MindDiagnosticReason::Kind::Memory && reason.eventId == 13;
    }));
    assert(std::any_of(diagnostic.reasons.begin(), diagnostic.reasons.end(), [](const auto& reason) {
        return reason.kind == MindDiagnosticReason::Kind::UnmetNeed && reason.need == NeedType::Sleep;
    }));
    assert(!diagnostic.interventions.empty());
    const std::string text = formatMindDiagnostic(diagnostic);
    assert(text.find("founder_death") != std::string::npos);
    assert(text.find("sleep") != std::string::npos);
    assert(text.find("work-blocked=crisis") != std::string::npos);

    // Recovery is an explicit state after severe stress once immediate causes
    // have been addressed, rather than pretending the traumatic memory vanished.
    CitizenMindSystem::updateNeed(citizen, NeedType::Sleep, 1.0f, 6'000, 15);
    CitizenMindSystem::updateNeed(citizen, NeedType::Safety, 1.0f, 7'000, 16);
    citizen.stress.accumulated = 40.0f;
    auto recovering = CitizenMindSystem::tick(citizen, 8'000, 1'000);
    assert(recovering.stressBandChanged);
    assert(citizen.stress.band == StressBand::Recovery);
    assert(!citizen.memories.entries.empty());

    citizen.stress.accumulated = 10.0f;
    auto stable = CitizenMindSystem::tick(citizen, 9'000, 1'000);
    assert(stable.stressBandChanged);
    assert(citizen.stress.band == StressBand::Stable);
}

void testTraitDependentCrisisResponses() {
    {
        auto s = makeCitizen(2001);
        s.personality.facets.fill(0.0f);
        s.personality.facets[static_cast<std::size_t>(PersonalityFacet::Anger)] = 1.0f;
        s.personality.facets[static_cast<std::size_t>(PersonalityFacet::RiskTolerance)] = 0.8f;
        assert(CitizenMindSystem::selectCrisisResponse(s) == CrisisResponse::Rage);
    }
    {
        auto s = makeCitizen(2002);
        s.personality.facets.fill(0.0f);
        s.personality.facets[static_cast<std::size_t>(PersonalityFacet::Anxiety)] = 1.0f;
        s.personality.facets[static_cast<std::size_t>(PersonalityFacet::RiskTolerance)] = 0.0f;
        assert(CitizenMindSystem::selectCrisisResponse(s) == CrisisResponse::Flight);
    }
    {
        auto s = makeCitizen(2003);
        s.personality.facets.fill(0.0f);
        s.personality.facets[static_cast<std::size_t>(PersonalityFacet::Curiosity)] = 1.0f;
        s.personality.facets[static_cast<std::size_t>(PersonalityFacet::Discipline)] = 1.0f;
        s.personality.facets[static_cast<std::size_t>(PersonalityFacet::Orderliness)] = 1.0f;
        assert(CitizenMindSystem::selectCrisisResponse(s) == CrisisResponse::Obsession);
    }
}

void testRelationshipSparseGraphAndPersistence() {
    RelationshipStore relationships;
    std::string error;
    const SimTimeMs now = 10'000;
    assert(relationships.linkParentChild(1, 2, now, 101, &error));
    assert(relationships.linkPartner(1, 3, now, 102, &error));
    assert(relationships.linkFriend(2, 4, now, 103, &error));
    assert(relationships.linkRivals(4, 5, now, 104, &error));
    assert(relationships.linkMentor(3, 4, now, 105, &error));
    assert(relationships.linkGuardianWard(3, 2, now, 106, &error));
    assert(relationships.linkCoworkers(1, 5, now, 108, &error));
    assert(relationships.linkSiblings(2, 5, now, 109, &error));

    // Sparse directed edges only: 15 edges for five citizens, not a 25-cell matrix.
    // Mentor is intentionally one directional edge because the documented type
    // vocabulary has Mentor but no fake reverse Mentee type.
    assert(relationships.size() == 15);
    assert(relationships.find(1, 2));
    assert(relationships.find(2, 1));
    assert((relationships.find(1, 2)->typeTags & relationshipMask(RelationshipType::Parent)) != 0);
    assert((relationships.find(2, 1)->typeTags & relationshipMask(RelationshipType::Child)) != 0);

    // Family ties do not passively decay; ordinary friendship does.
    const SimTimeMs later = now + 200LL * 24LL * 60LL * 60LL * 1000LL;
    const auto parentLater = relationships.effective(1, 2, later);
    const auto friendLater = relationships.effective(2, 4, later);
    assert(parentLater && friendLater);
    assert(near(parentLater->familiarity, relationships.find(1, 2)->familiarity));
    assert(friendLater->familiarity < relationships.find(2, 4)->familiarity);

    // Relationship dimensions are bounded event deltas, with grievance emitting
    // an explicit social event rather than an invisible side effect.
    relationships.clearEmittedEvents();
    RelationshipDelta grievance{};
    grievance.grievance = 0.3f;
    grievance.affinity = -0.2f;
    assert(relationships.apply(2, 4, grievance, now + 1'000, 107, &error));
    assert(std::any_of(relationships.emittedEvents().begin(), relationships.emittedEvents().end(), [](const auto& e) {
        return e.type == SocialDomainEventType::GrievanceCreated;
    }));

    std::vector<std::uint8_t> bytes;
    assert(encodeRelationshipStore(relationships, bytes, &error));
    assert(!bytes.empty());
    RelationshipStore loaded;
    assert(decodeRelationshipStore(bytes, loaded, &error));
    assert(loaded.allEdges() == relationships.allEdges());

    // Runtime entity handles never participate: the graph is recoverable from
    // stable IDs alone after unload/migration/death of a live ECS instance.
    assert(loaded.find(1, 3));
    assert(loaded.find(4, 5));
    assert(loaded.find(3, 4));
    assert(!loaded.find(4, 3));
    assert((loaded.find(1, 5)->typeTags & relationshipMask(RelationshipType::Coworker)) != 0);
    assert((loaded.find(2, 5)->typeTags & relationshipMask(RelationshipType::Sibling)) != 0);

    // Negative contracts.
    RelationshipEdge bad{};
    bad.source = 9;
    bad.target = 9;
    bad.typeTags = relationshipMask(RelationshipType::Friend);
    assert(!loaded.upsert(bad, &error));
    RelationshipDelta tooLarge{};
    tooLarge.affinity = 0.75f;
    assert(!loaded.apply(1, 2, tooLarge, now, 0, &error));
}


void testFamilyGenerationCanonicalAndGeneric() {
    RelationshipStore a;
    RelationshipStore b;
    std::string error;
    const SimTimeMs now = 55'000;

    const std::vector<CitizenStableId> parentsA{30, 10, 30, 20};
    const std::vector<CitizenStableId> siblingsA{60, 50, 60};
    const std::vector<CitizenStableId> guardiansA{40, 20, 40}; // parent 20 wins over duplicate guardian role

    const std::vector<CitizenStableId> parentsB{20, 30, 10};
    const std::vector<CitizenStableId> siblingsB{50, 60};
    const std::vector<CitizenStableId> guardiansB{40};

    assert(a.linkFamilyGeneration(99, parentsA, siblingsA, guardiansA, now, 5001, &error));
    assert(b.linkFamilyGeneration(99, parentsB, siblingsB, guardiansB, now, 5001, &error));
    assert(a.allEdges() == b.allEdges());

    // Three parents, one guardian and two siblings are legal: Agent 18 does not
    // hard-code a human two-parent family assumption. Links are reciprocal except
    // where the vocabulary itself is directional.
    for (CitizenStableId parent : {10ull, 20ull, 30ull}) {
        assert(a.find(parent, 99));
        assert(a.find(99, parent));
        assert((a.find(parent, 99)->typeTags & relationshipMask(RelationshipType::Parent)) != 0);
        assert((a.find(99, parent)->typeTags & relationshipMask(RelationshipType::Child)) != 0);
    }
    assert(a.find(40, 99));
    assert(a.find(99, 40));
    assert((a.find(40, 99)->typeTags & relationshipMask(RelationshipType::Guardian)) != 0);
    assert((a.find(99, 40)->typeTags & relationshipMask(RelationshipType::Ward)) != 0);
    for (CitizenStableId sibling : {50ull, 60ull}) {
        assert((a.find(99, sibling)->typeTags & relationshipMask(RelationshipType::Sibling)) != 0);
        assert((a.find(sibling, 99)->typeTags & relationshipMask(RelationshipType::Sibling)) != 0);
    }

    // Stable sort/canonicalization makes persistence independent of caller order.
    std::vector<std::uint8_t> bytesA, bytesB;
    assert(encodeRelationshipStore(a, bytesA, &error));
    assert(encodeRelationshipStore(b, bytesB, &error));
    assert(bytesA == bytesB);

    // Bad generation input is rejected before any edge is published.
    RelationshipStore rejected;
    const std::vector<CitizenStableId> invalidParents{1, 0, 2};
    assert(!rejected.linkFamilyGeneration(99, invalidParents, {}, {}, now, 5002, &error));
    assert(rejected.size() == 0);
    const std::vector<CitizenStableId> selfSibling{99};
    assert(!rejected.linkFamilyGeneration(99, {}, selfSibling, {}, now, 5003, &error));
    assert(rejected.size() == 0);
}

void testCitizenMindPersistenceAndCorruptionChecks() {
    auto citizen = makeCitizen(3001);
    std::string error;
    assert(CitizenMindSystem::setPreference(citizen.preferences,
        {PreferenceCategory::Material, "elysium:material/voidglass", 0.8f}, &error));
    assert(CitizenMindSystem::setPreference(citizen.preferences,
        {PreferenceCategory::Activity, "elysium:activity/research", 0.7f}, &error));

    CitizenMindSystem::updateNeed(citizen, NeedType::Learning, 0.1f, 1000, 201);
    MindEvent discovery{};
    discovery.eventId = 202;
    discovery.time = 2000;
    discovery.type = MindEventType::Discovery;
    discovery.valence = 0.9f;
    discovery.intensity = 0.9f;
    discovery.chronicleImportance = 0.8f;
    discovery.topic = "elysium:discovery/rift_signature";
    discovery.reaction.facet = PersonalityFacet::Curiosity;
    discovery.reaction.facetWeight = 1.0f;
    discovery.reaction.value = ValueDimension::Knowledge;
    discovery.reaction.valueWeight = 1.0f;
    CitizenMindSystem::applyEvent(citizen, discovery);

    CitizenMindRecord record{};
    record.state = citizen;
    std::vector<std::uint8_t> bytes;
    assert(encodeCitizenMindRecord(record, bytes, &error, true));
    CitizenMindRecord loaded{};
    assert(decodeCitizenMindRecord(bytes, loaded, &error));
    assert(loaded.state.stableId == citizen.stableId);
    assert(loaded.state.preferences.entries == citizen.preferences.entries);
    assert(loaded.state.memories.entries == citizen.memories.entries);
    assert(loaded.state.thoughts.entries == citizen.thoughts.entries);
    assert(near(loaded.state.stress.accumulated, citizen.stress.accumulated));
    assert(near(loaded.state.focus.value, CitizenMindSystem::recomputeFocus(loaded.state)));

    auto corrupted = bytes;
    corrupted.resize(corrupted.size() - 3);
    CitizenMindRecord rejected{};
    assert(!decodeCitizenMindRecord(corrupted, rejected, &error));

    // CurrentThoughts are conditional persistence; remote compaction can omit
    // them while durable Memories and StressState remain.
    std::vector<std::uint8_t> remoteBytes;
    assert(encodeCitizenMindRecord(record, remoteBytes, &error, false));
    CitizenMindRecord remote{};
    assert(decodeCitizenMindRecord(remoteBytes, remote, &error));
    assert(remote.state.thoughts.entries.empty());
    assert(remote.state.memories.entries == citizen.memories.entries);
}

std::vector<CitizenMindState> makeBatch() {
    std::vector<CitizenMindState> states;
    for (std::uint64_t i = 0; i < 96; ++i) {
        auto s = makeCitizen(50'000 + (95 - i)); // deliberately reverse identity order
        s.personality.facets[static_cast<std::size_t>(PersonalityFacet::Anxiety)] = static_cast<float>((i * 7) % 100) / 100.0f;
        s.personality.facets[static_cast<std::size_t>(PersonalityFacet::Discipline)] = static_cast<float>((i * 13) % 100) / 100.0f;
        s.needs.dimensions[static_cast<std::size_t>(NeedType::Sleep)].satisfaction = static_cast<float>((i * 11) % 100) / 100.0f;
        s.needs.dimensions[static_cast<std::size_t>(NeedType::Food)].satisfaction = static_cast<float>((i * 17) % 100) / 100.0f;
        MindEvent e{};
        e.eventId = 90'000 + i;
        e.time = 1000;
        e.type = (i % 2) ? MindEventType::WorkFailure : MindEventType::Achievement;
        e.valence = (i % 2) ? -0.8f : 0.7f;
        e.intensity = 0.4f + 0.005f * static_cast<float>(i % 50);
        e.chronicleImportance = 0.2f;
        e.topic = "elysium:test/scripted";
        CitizenMindSystem::applyEvent(s, e);
        states.push_back(std::move(s));
    }
    return states;
}

std::vector<std::uint8_t> runBatchWithWorkers(int workers) {
    auto states = makeBatch();
    if (workers == 0) {
        JobSystem jobs(SerialJobs);
        CitizenMindSystem::tickBatch(jobs, states, 61'000, 60'000);
    } else {
        JobSystem jobs(static_cast<std::size_t>(workers));
        CitizenMindSystem::tickBatch(jobs, states, 61'000, 60'000);
    }
    std::string error;
    const auto bytes = encodeMindBatchCanonical(states, &error);
    assert(!bytes.empty());
    return bytes;
}

void testWorkerCountDeterminism() {
    const auto serial = runBatchWithWorkers(0);
    for (int workers : {1, 2, 4, 8}) {
        const auto candidate = runBatchWithWorkers(workers);
        assert(candidate == serial);
    }
}

void testIdenticalScriptDeterminism() {
    auto a = makeCitizen(4001);
    auto b = makeCitizen(4001);
    for (int i = 0; i < 20; ++i) {
        MindEvent e{};
        e.eventId = 1000 + static_cast<std::uint64_t>(i);
        e.time = i * 1000;
        e.type = (i % 3 == 0) ? MindEventType::Conflict : MindEventType::SocialInteraction;
        e.valence = (i % 3 == 0) ? -0.7f : 0.4f;
        e.intensity = 0.6f;
        e.chronicleImportance = (i == 9) ? 0.9f : 0.1f;
        e.other = 9999;
        e.topic = "elysium:test/interaction";
        e.reaction.facet = PersonalityFacet::Sociability;
        e.reaction.facetWeight = 0.5f;
        CitizenMindSystem::applyEvent(a, e);
        CitizenMindSystem::applyEvent(b, e);
    }
    CitizenMindSystem::tick(a, 30'000, 10'000);
    CitizenMindSystem::tick(b, 30'000, 10'000);
    std::string error;
    const auto ba = encodeMindBatchCanonical({a}, &error);
    const auto bb = encodeMindBatchCanonical({b}, &error);
    assert(ba == bb);
}

} // namespace

int main() {
    testNeedThoughtMemoryStressFocusAndDiagnostics();
    testTraitDependentCrisisResponses();
    testRelationshipSparseGraphAndPersistence();
    testFamilyGenerationCanonicalAndGeneric();
    testCitizenMindPersistenceAndCorruptionChecks();
    testWorkerCountDeterminism();
    testIdenticalScriptDeterminism();
    std::cout << "citizen mind/relationship tests passed\n";
    return 0;
}
