// Intended function: imported tests implementation for justice_security_tests; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/JusticeSecurity.hpp"
#include "world/PlanetSurface.hpp"
#include "world/SurfaceInfrastructure.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace elysium;

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

void testJusticeSecurityCrimeEvidenceAccessAndPersistence() {
    JusticeTuning tuning;
    tuning.evidenceSufficientScore=1.40f;
    tuning.independentSourcesForSufficiency=2;
    tuning.contaminatedEvidenceFactor=0.40f;
    SecurityArchive archive(0x27C0FFEEULL,tuning);
    CrimeDetectionSystem detection(archive);
    InvestigationSystem investigation(archive);
    JusticeSystem justice(archive);
    CustodySystem custody(archive);
    InfiltrationSystem infiltration(archive);

    constexpr SecurityStableId actualActor=0xA11CEULL;
    constexpr SecurityStableId mistakenActor=0xBADF00DULL;
    constexpr SecurityStableId victim=0xBEEF100ULL;
    constexpr SecurityStableId witness=0x10001ULL;
    constexpr SecurityStableId witness2=0x10002ULL;
    constexpr SecurityStableId sensor=0x20002ULL;
    constexpr SecurityStableId investigatorId=0x30003ULL;
    constexpr SecurityStableId judgeId=0x40004ULL;
    constexpr SecurityStableId evidenceLocker=0x50005ULL;
    constexpr SecurityStableId detentionBlock=0x60006ULL;

    // Hidden truth exists in simulation state, but no case may exist until
    // actual witness/sensor/physical evidence reaches authorities.
    const auto incidentId=archive.createIncident(CrimeType::Theft,actualActor,{victim},
        SecurityLocation{0xF0A7ULL,0x77ULL,12,4,9},100,HistoryPolicy::Major);
    require(incidentId!=0,"crime incident failed to allocate stable ID");
    require(archive.cases().empty() && archive.evidenceRecords().empty(),
            "unobserved crime leaked omniscient case/evidence state");

    SensorObservationInput darkSensor;
    darkSensor.sensorStableId=sensor;
    darkSensor.observedSubjectStableId=actualActor;
    darkSensor.powered=false;
    darkSensor.hadCoverage=true;
    darkSensor.confidence=1.0f;
    require(detection.submitSensorObservation(incidentId,darkSensor,101)==0,
            "unpowered sensor generated evidence");
    require(archive.cases().empty(),"unpowered sensor exposed hidden crime");

    // A mistaken witness can create a real but fallible case against the wrong
    // person; hidden incident truth is deliberately not consulted.
    WitnessStatementInput mistaken;
    mistaken.witnessStableId=witness;
    mistaken.believedSubjectStableId=mistakenActor;
    mistaken.supportsAllegation=true;
    mistaken.perceptionQuality=0.95f;
    mistaken.memoryFidelity=0.90f;
    mistaken.bias=0.15f;
    mistaken.loyalty=0.20f;
    mistaken.fear=0.10f;
    const auto witnessEvidence=detection.submitWitnessStatement(incidentId,mistaken,102);
    require(witnessEvidence!=0 && archive.cases().size()==1,
            "witness report did not create one evidence-backed case");
    const auto caseId=archive.cases().front().stableId;
    require(archive.cases().front().suspectStableIds.size()==1 &&
            archive.cases().front().suspectStableIds.front()==mistakenActor,
            "case silently substituted objective perpetrator for witness belief");

    // Repeating an identical report is idempotent and cannot manufacture
    // duplicate evidence weight or a second case.
    const auto repeated=detection.submitWitnessStatement(incidentId,mistaken,103);
    require(repeated==witnessEvidence && archive.cases().size()==1 && archive.evidenceRecords().size()==1,
            "duplicate witness report corrupted case/evidence identity");

    WitnessStatementInput conflicting=mistaken;
    conflicting.witnessStableId=witness2;
    conflicting.supportsAllegation=false;
    conflicting.perceptionQuality=0.85f;
    conflicting.memoryFidelity=0.80f;
    conflicting.bias=-0.10f;
    const auto conflictingEvidence=detection.submitWitnessStatement(incidentId,conflicting,103);
    require(conflictingEvidence!=0,"conflicting witness statement was not recorded");
    const auto conflictingAssessment=investigation.assess(caseId,mistakenActor);
    require(conflictingAssessment.supportingEvidence==1 && conflictingAssessment.conflictingEvidence==1 &&
            conflictingAssessment.exculpatoryScore>0.0f,
            "case could not represent conflicting testimony about the same suspect");

    SensorObservationInput poweredSensor;
    poweredSensor.sensorStableId=sensor;
    poweredSensor.observedSubjectStableId=actualActor;
    poweredSensor.powered=true;
    poweredSensor.hadCoverage=true;
    poweredSensor.confidence=0.88f;
    const auto sensorEvidence=detection.submitSensorObservation(incidentId,poweredSensor,104);
    require(sensorEvidence!=0 && archive.cases().size()==1,
            "independent sensor evidence created duplicate case");
    require(archive.caseForIncident(incidentId)->stableId==caseId,
            "same incident no longer resolves to original case");

    // Physical evidence is only added after an investigator actually discovers
    // it. Together with the sensor it is sufficient to formally accuse.
    const auto physical=investigation.addPhysicalEvidence(incidentId,EvidenceKind::ItemProvenance,
        evidenceLocker,actualActor,true,0.92f,105);
    require(physical!=0,"physical evidence discovery failed");
    require(investigation.assignInvestigator(caseId,investigatorId),"investigator assignment failed");
    const auto actualAssessment=investigation.assess(caseId,actualActor);
    require(actualAssessment.sufficient && actualAssessment.independentSources==2,
            "independent sensor + physical evidence did not meet configured sufficiency gate");
    require(investigation.refreshEvidenceStatus(caseId) &&
            archive.securityCase(caseId)->status==CaseStatus::EvidenceSufficient,
            "case did not expose EvidenceSufficient status after evidence gate was met");
    const auto wrongAssessment=investigation.assess(caseId,mistakenActor);
    require(!wrongAssessment.sufficient,"single mistaken statement became sufficient under two-source policy");

    // Evidence integrity changes the case without deleting provenance.
    require(investigation.contaminateEvidence(sensorEvidence),"sensor evidence contamination failed");
    require(!investigation.assess(caseId,actualActor).sufficient,
            "contaminated evidence retained full evidentiary weight");
    require(investigation.refreshEvidenceStatus(caseId) &&
            archive.securityCase(caseId)->status==CaseStatus::Interviewing,
            "case did not fall back from EvidenceSufficient after evidence contamination");
    const auto accessEvidence=investigation.addPhysicalEvidence(incidentId,EvidenceKind::AccessRecord,
        0x5151ULL,actualActor,true,0.90f,106);
    require(accessEvidence!=0 && investigation.assess(caseId,actualActor).sufficient,
            "additional independent access evidence did not restore sufficiency");
    require(investigation.refreshEvidenceStatus(caseId) &&
            archive.securityCase(caseId)->status==CaseStatus::EvidenceSufficient,
            "case sufficiency status did not recover after replacement evidence");
    require(investigation.loseEvidence(witnessEvidence),"witness evidence loss failed");
    require(investigation.assess(caseId,mistakenActor).supportingScore==0.0f,
            "lost mistaken statement still influenced accusation score");

    require(investigation.accuse(caseId,actualActor,investigatorId,107),
            "evidence-sufficient accusation failed");
    require(archive.securityCase(caseId)->status==CaseStatus::Adjudication &&
            archive.securityCase(caseId)->accusedStableId==actualActor,
            "case did not enter adjudication with stable accused ID");
    require(justice.adjudicate(caseId,VerdictKind::Convicted,JusticeAction::Detention,judgeId,108),
            "conviction/detention verdict failed");
    require(custody.applyCaseAction(caseId,detentionBlock,109),"custody action did not materialize prisoner state");
    require(archive.custodyFor(actualActor)!=nullptr && archive.custodyFor(actualActor)->active,
            "detention state missing after conviction");
    require(justice.pardon(caseId,judgeId,110),"pardon failed");
    require(!archive.custodyFor(actualActor)->active && archive.securityCase(caseId)->verdict==VerdictKind::Pardoned,
            "pardon did not release custody/update case history");

    // Timed custody ends deterministically; indefinite custody is not released
    // by the expiry sweep. Use a second case so the pardon path remains intact.
    const auto timedIncident=archive.createIncident(CrimeType::Contraband,0xC0A7ULL,{},
        SecurityLocation{0xF0A7ULL,0x77ULL,2,3,4},111,HistoryPolicy::Ordinary);
    const auto timedEvidence=investigation.addPhysicalEvidence(timedIncident,EvidenceKind::Contraband,
        0xE71DULL,0xC0A7ULL,true,1.0f,111);
    const auto timedEvidence2=investigation.addPhysicalEvidence(timedIncident,EvidenceKind::ItemProvenance,
        0xE71EULL,0xC0A7ULL,true,1.0f,111);
    const auto timedCase=archive.caseForIncident(timedIncident)->stableId;
    require(timedEvidence!=0 && timedEvidence2!=0 && investigation.accuse(timedCase,0xC0A7ULL,investigatorId,112),
            "timed custody fixture accusation failed");
    require(justice.adjudicate(timedCase,VerdictKind::Convicted,JusticeAction::Detention,judgeId,113),
            "timed custody fixture verdict failed");
    require(custody.applyCaseAction(timedCase,detentionBlock,113,120),"timed custody fixture apply failed");
    require(custody.releaseExpired(119)==0 && archive.custodyFor(0xC0A7ULL)->active,
            "custody expired before configured end");
    require(custody.releaseExpired(120)==1 && !archive.custodyFor(0xC0A7ULL)->active,
            "custody did not release exactly at configured end");

    require(archive.historyEvents().size()>=4,
            "major justice outcomes were not appended to persistent history");

    // Restricted access is policy/credential driven. The target is an actual
    // stable Door object from SurfaceInfrastructure, but AccessControlSystem
    // neither knows nor switches on faction identities.
    PlanetSurface planet(0xACC355ULL,PlanetClass::Temperate);
    SurfaceInfrastructure infrastructure(planet.seed());
    const SurfaceCellAddress doorCell{CubeFace::PositiveZ,20,20,PlanetSurface::ReferenceRadial+2};
    planet.set(doorCell,BlockType::Air,true);
    const auto doorId=infrastructure.placePortal(planet,SurfacePortalType::Door,doorCell,false);
    require(doorId!=0,"access-control fixture could not create stable Door target");

    AccessPolicy policy;
    policy.stableId=0xA001ULL;
    policy.targetKind=AccessTargetKind::Door;
    policy.targetStableId=doorId;
    policy.minimumClearance=2;
    policy.requiredCredentialTags={7,42};
    policy.allowedLegalStatusMask=(1u<<static_cast<unsigned>(LegalStatus::Resident)) |
                                  (1u<<static_cast<unsigned>(LegalStatus::Citizen));
    policy.auditDenied=true;
    policy.auditAllowed=true;

    AccessControlSystem access;
    AccessCredential citizen;
    citizen.subjectStableId=0xC1712EULL;
    citizen.legalStatus=LegalStatus::Citizen;
    citizen.clearanceLevel=2;
    citizen.credentialTags={42,7};
    const auto allowed=access.evaluate(citizen,policy,111);
    require(allowed.allowed && allowed.reason=="policy satisfied","valid security credential was denied");

    AccessCredential visitor=citizen;
    visitor.subjectStableId=0x7151702ULL;
    visitor.legalStatus=LegalStatus::Visitor;
    require(!access.evaluate(visitor,policy,112).allowed,"visitor bypassed restricted-area legal-status policy");
    require(access.auditLog().size()==2,"door access audit did not record configured decisions");

    // Access logs become evidence only when an actual audited event is
    // deliberately associated with a real incident. Merely having a door log
    // never opens a case on its own.
    const auto accessIncident=archive.createIncident(CrimeType::Espionage,visitor.subjectStableId,{},
        SecurityLocation{0xF0A7ULL,doorId,20,20,PlanetSurface::ReferenceRadial+2},112,HistoryPolicy::Ordinary);
    require(archive.caseForIncident(accessIncident)==nullptr,"access audit leaked into a case without detection submission");
    const auto auditEvidence=detection.submitAccessAuditObservation(accessIncident,access.auditLog().back(),
        visitor.subjectStableId,true,0.8f);
    require(auditEvidence!=0 && archive.evidence(auditEvidence)->kind==EvidenceKind::AccessRecord,
            "access audit did not translate into access-record evidence");
    require(detection.submitAccessAuditObservation(accessIncident,access.auditLog().back(),
        visitor.subjectStableId,true,0.8f)==auditEvidence,
            "duplicate access audit manufactured duplicate evidence");

    // A false cover identity can pass access policy until actual evidence
    // reveals it. Counterintelligence never consults a random reveal roll.
    InfiltrationState infiltrator;
    infiltrator.actorStableId=0x1F117A70ULL;
    infiltrator.kind=InfiltratorKind::EngineeredAgent;
    infiltrator.coverIdentityStableId=citizen.subjectStableId;
    infiltrator.hiddenGoalStableId=0x51A80ULL;
    infiltrator.revealThreshold=1.35f;
    require(infiltration.registerInfiltrator(infiltrator),"infiltrator registration failed");
    require(archive.infiltration(infiltrator.actorStableId)->detectionState==InfiltrationDetectionState::Hidden,
            "infiltrator was revealed without evidence");

    AccessCredential coverCredential=citizen;
    coverCredential.subjectStableId=infiltrator.actorStableId;
    coverCredential.presentedIdentityStableId=infiltrator.coverIdentityStableId;
    require(access.evaluate(coverCredential,policy,113).allowed,
            "cover identity was omnisciently rejected before counterintelligence evidence");

    const auto infiltrationIncident=archive.createIncident(CrimeType::Espionage,infiltrator.actorStableId,{},
        SecurityLocation{0xF0A7ULL,0x88ULL,14,7,9},114,HistoryPolicy::Major);
    require(infiltrationIncident!=0 && archive.caseForIncident(infiltrationIncident)==nullptr,
            "unobserved infiltration incident leaked a case before evidence");
    const auto medical=investigation.addPhysicalEvidence(infiltrationIncident,EvidenceKind::MedicalScan,
        0x5ED1ULL,infiltrator.actorStableId,true,0.75f,114);
    const auto behavior=investigation.addPhysicalEvidence(infiltrationIncident,EvidenceKind::BehavioralObservation,
        0xB3A7ULL,infiltrator.actorStableId,true,0.70f,115);
    require(medical!=0 && behavior!=0,"counterintelligence evidence fixture failed");
    require(!infiltration.observeEvidence(infiltrator.actorStableId,medical,114),
            "single sub-threshold observation revealed infiltrator");
    require(archive.infiltration(infiltrator.actorStableId)->detectionState==InfiltrationDetectionState::Suspected,
            "single observation did not produce suspected state");
    require(investigation.loseEvidence(medical),"counterintelligence evidence loss failed");
    require(infiltration.reevaluateEvidence(infiltrator.actorStableId) &&
            archive.infiltration(infiltrator.actorStableId)->detectionState==InfiltrationDetectionState::Hidden,
            "lost pre-reveal evidence did not clear provisional suspicion");
    const auto replacementMedical=investigation.addPhysicalEvidence(infiltrationIncident,EvidenceKind::MedicalScan,
        0x5ED2ULL,infiltrator.actorStableId,true,0.75f,115);
    require(replacementMedical!=0 && !infiltration.observeEvidence(infiltrator.actorStableId,replacementMedical,115),
            "replacement sub-threshold evidence unexpectedly revealed infiltrator");
    require(infiltration.observeEvidence(infiltrator.actorStableId,behavior,116),
            "independent evidence did not reveal infiltrator at configured threshold");
    require(archive.infiltration(infiltrator.actorStableId)->detectionState==InfiltrationDetectionState::Revealed,
            "revealed infiltrator state was not persisted in archive");

    // Save/load is stable-ID based and deliberately does not require live actor
    // existence. The arbitrary accused/witness/infiltrator IDs therefore model
    // absent or remote historical figures across shard demotion.
    const auto serialized=archive.serializeState();
    SecurityArchive restored(0);
    std::string error;
    require(restored.restoreState(serialized,&error),"justice/security state failed round trip: "+error);
    const auto* restoredCase=restored.securityCase(caseId);
    require(restoredCase!=nullptr && restoredCase->accusedStableId==actualActor &&
            restoredCase->verdict==VerdictKind::Pardoned && restoredCase->outcomeHistory.size()==2,
            "case save/load lost stable accused/verdict/outcome history");
    require(restored.infiltration(infiltrator.actorStableId)!=nullptr &&
            restored.infiltration(infiltrator.actorStableId)->detectionState==InfiltrationDetectionState::Revealed,
            "infiltration state did not survive save/load");
    require(restored.serializeState()==serialized,
            "justice/security codec is not deterministic after round trip");

    // Corruption/dangling references fail loudly instead of manufacturing
    // actors or silently discarding records.
    auto corrupted=serialized;
    const auto pos=corrupted.find("END\n");
    require(pos!=std::string::npos,"justice/security fixture missing END marker");
    corrupted.replace(pos,4,"BROKEN\n");
    require(!restored.restoreState(corrupted,&error),"malformed justice/security state was accepted");
}

} // namespace

int main() {
    try {
        testJusticeSecurityCrimeEvidenceAccessAndPersistence();
        std::cout << "Elysium justice/security tests: PASS\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Elysium justice/security tests: FAIL: " << e.what() << '\n';
        return 1;
    }
}
