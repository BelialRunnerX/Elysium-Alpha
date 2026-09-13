// Intended function: imported tests implementation for medical_tests; preserves the agent-authored subsystem contract for later integration/debugging.
#include "medical/MedicalSystem.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using namespace elysium::medical;

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

struct ResourceKey {
    ResourceKind kind{};
    StableId stableId{};
    std::uint32_t contentId{};

    friend bool operator<(const ResourceKey& a, const ResourceKey& b) {
        return std::tuple{static_cast<int>(a.kind), a.stableId, a.contentId} <
               std::tuple{static_cast<int>(b.kind), b.stableId, b.contentId};
    }
};

// Narrow test adapter for the Agent 19/21 reservation seam. It deliberately
// knows nothing about medical behavior; it only proves all-or-nothing claims,
// exclusivity and bounded consumable quantities.
class TestReservationGateway final : public IMedicalReservationGateway {
public:
    void stock(MedicalSupply supply, int count) {
        stock_[static_cast<std::uint32_t>(supply)] = count;
    }

    ReservationAttempt tryReserveAll(StableId jobStableId,
                                     std::span<const ResourceRequest> requests) override {
        if (jobStableId == InvalidStableId) return {false,"reservation owner is invalid"};
        if (claimsByJob_.contains(jobStableId)) return {false,"job already owns a reservation set"};

        std::map<ResourceKey,int> localSupply;
        for (const auto& request : requests) {
            if (request.quantity <= 0) return {false,"reservation quantity must be positive"};
            const ResourceKey key{request.kind,request.stableId,request.contentId};
            if (request.exclusive) {
                if (request.stableId == InvalidStableId) return {false,"exclusive resource requires stable identity"};
                const auto owner=exclusiveOwner_.find(key);
                if (owner!=exclusiveOwner_.end() && owner->second!=jobStableId)
                    return {false,"exclusive medical resource already reserved"};
            } else if (request.kind==ResourceKind::Supply) {
                localSupply[key]+=request.quantity;
            }
        }

        for (const auto& [key,quantity] : localSupply) {
            int already=0;
            for (const auto& [job,claims] : claimsByJob_) {
                (void)job;
                for (const auto& claim : claims)
                    if (!claim.exclusive && claim.kind==ResourceKind::Supply &&
                        claim.contentId==key.contentId) already+=claim.quantity;
            }
            const int available=stock_[key.contentId];
            if (already+quantity>available) return {false,"insufficient unreserved medical supplies"};
        }

        std::vector<ResourceRequest> claims(requests.begin(),requests.end());
        claimsByJob_.emplace(jobStableId,claims);
        for (const auto& claim:claims) if(claim.exclusive)
            exclusiveOwner_[{claim.kind,claim.stableId,claim.contentId}]=jobStableId;
        return {true,{}};
    }

    bool commitConsumables(StableId jobStableId) override {
        const auto it=claimsByJob_.find(jobStableId);
        if(it==claimsByJob_.end()) return false;
        std::map<std::uint32_t,int> need;
        for(const auto& claim:it->second)
            if(!claim.exclusive && claim.kind==ResourceKind::Supply) need[claim.contentId]+=claim.quantity;
        for(const auto& [content,count]:need) if(stock_[content]<count) return false;
        for(const auto& [content,count]:need) stock_[content]-=count;
        return true;
    }

    void release(StableId jobStableId) override {
        const auto it=claimsByJob_.find(jobStableId);
        if(it==claimsByJob_.end()) return;
        for(const auto& claim:it->second) if(claim.exclusive) {
            const ResourceKey key{claim.kind,claim.stableId,claim.contentId};
            const auto owner=exclusiveOwner_.find(key);
            if(owner!=exclusiveOwner_.end() && owner->second==jobStableId) exclusiveOwner_.erase(owner);
        }
        claimsByJob_.erase(it);
    }

    bool isExclusiveHeld(ResourceKind kind,StableId stableId) const {
        return exclusiveOwner_.contains({kind,stableId,0});
    }

    int stockRemaining(MedicalSupply supply) const {
        const auto it=stock_.find(static_cast<std::uint32_t>(supply));
        return it==stock_.end()?0:it->second;
    }

private:
    std::map<ResourceKey,StableId> exclusiveOwner_;
    std::unordered_map<StableId,std::vector<ResourceRequest>> claimsByJob_;
    std::unordered_map<std::uint32_t,int> stock_;
};

MedicalWorkerProfile expertWorker(StableId id=7001) {
    MedicalWorkerProfile worker;
    worker.stableId=id;
    worker.skills.fill(1.0f);
    return worker;
}

TreatmentExecutionContext contextFor(StableId jobId, const TreatmentTask& task, StableId deviceId=9001) {
    TreatmentExecutionContext c;
    c.jobStableId=jobId;
    c.worker=expertWorker();
    c.bedStableId=8001;
    c.facilityStableId=8101;
    c.deviceStableId=deviceId;
    c.prostheticKind=ProstheticKind::MechanicalLimb;
    (void)task;
    return c;
}

void fillStock(TestReservationGateway& gateway, int count=64) {
    for (int i=static_cast<int>(MedicalSupply::Bandage); i<=static_cast<int>(MedicalSupply::ProstheticParts); ++i)
        gateway.stock(static_cast<MedicalSupply>(i),count);
}

PatientMedicalState makeTraumaFixture(StableId patientId) {
    const auto& plan=standardHumanoidBodyPlan();
    auto patient=MedicalSystem::makePatient(patientId,plan);
    const auto leftLeg=findPartByName(plan,"left_leg");
    const auto rightHand=findPartByName(plan,"right_hand");
    require(leftLeg && rightHand,"standard body plan missing fixture parts");

    InjurySpec fracture;
    fracture.partId=*leftLeg;
    fracture.kind=InjuryKind::Fracture;
    fracture.severity=InjurySeverity::FunctionLoss;
    fracture.bleedPerMinute=0.0f;
    MedicalSystem::applyInjury(patient,plan,fracture);

    InjurySpec laceration;
    laceration.partId=*leftLeg;
    laceration.kind=InjuryKind::Laceration;
    laceration.severity=InjurySeverity::Inhibited;
    laceration.bleedPerMinute=-1.0f;
    laceration.contamination=0.25f;
    MedicalSystem::applyInjury(patient,plan,laceration);

    InjurySpec amputation;
    amputation.partId=*rightHand;
    amputation.kind=InjuryKind::Amputation;
    amputation.severity=InjurySeverity::Missing;
    amputation.bleedPerMinute=-1.0f;
    MedicalSystem::applyInjury(patient,plan,amputation);
    return patient;
}

std::vector<std::tuple<int,WoundId,BodyPartId>> planSignature(const PatientMedicalState& patient) {
    const auto plan=MedicalSystem::treatmentPlan(patient,standardHumanoidBodyPlan());
    std::vector<std::tuple<int,WoundId,BodyPartId>> out;
    for(const auto& task:plan) out.emplace_back(static_cast<int>(task.kind),task.woundId,task.partId);
    return out;
}

void testBodyPlanAndCapabilityLoss() {
    const auto& plan=standardHumanoidBodyPlan();
    require(plan.contentId.rfind("elysium:",0)==0,"body plan content ID is not namespaced");
    auto healthy=MedicalSystem::makePatient(1001,plan);
    require(healthy.bodyParts.size()==plan.parts.size(),"patient did not instantiate one state per important body part");
    require(healthy.capabilities.get(Capability::Walk)>0.999f,"healthy walk capability not normalized");
    require(healthy.capabilities.get(Capability::Grasp)>0.999f,"healthy grasp capability not normalized");

    auto patient=makeTraumaFixture(1002);
    require(patient.capabilities.get(Capability::Walk)<0.90f,"fractured leg did not reduce effective mobility");
    require(patient.capabilities.get(Capability::Grasp)<0.75f,"missing hand did not reduce grasp capability");
    require(patient.medical.triage==TriageCategory::Immediate,"severe multi-trauma was not triaged immediate");
    require(!patient.medical.readyForWork,"severe trauma left patient erroneously work eligible");
    require(MedicalSystem::scalarHealthEquivalent(patient,plan)<99.0f,"body-state injury did not bridge to scalar action health");
}

void testDirectActionDamageBridge() {
    const auto& plan=standardHumanoidBodyPlan();
    auto patient=MedicalSystem::makePatient(1101,plan);
    const auto torso=findPartByName(plan,"torso");
    require(torso.has_value(),"body plan has no torso");
    const float before=MedicalSystem::scalarHealthEquivalent(patient,plan);
    const auto events=MedicalSystem::applyDirectDamage(patient,plan,{42.0f,InjuryKind::Crush,*torso});
    require(!events.empty() && events.front().kind==MedicalEventKind::CitizenWounded,"direct damage emitted no persistent body event");
    require(MedicalSystem::findPartState(patient,*torso)->condition<0.60f,"direct scalar damage was not translated to part damage");
    require(MedicalSystem::scalarHealthEquivalent(patient,plan)<before,"direct damage did not reduce scalar compatibility health");
}

void testReservationAtomicityAndNoDoubleBook() {
    TestReservationGateway gateway;
    fillStock(gateway,3);
    const std::vector<ResourceRequest> first{
        {ResourceKind::Worker,7001,0,1,true},
        {ResourceKind::Bed,8001,0,1,true},
        {ResourceKind::Supply,0,static_cast<std::uint32_t>(MedicalSupply::Bandage),2,false}
    };
    const std::vector<ResourceRequest> conflicting{
        {ResourceKind::Worker,7002,0,1,true},
        {ResourceKind::Bed,8001,0,1,true},
        {ResourceKind::Supply,0,static_cast<std::uint32_t>(MedicalSupply::Bandage),2,false}
    };
    require(gateway.tryReserveAll(501,first).success,"first patient resource set could not reserve");
    const auto second=gateway.tryReserveAll(502,conflicting);
    require(!second.success,"second patient double-booked an occupied bed");
    require(!gateway.isExclusiveHeld(ResourceKind::Worker,7002),"failed atomic reservation leaked a staff claim");
    require(gateway.stockRemaining(MedicalSupply::Bandage)==3,"reservation consumed supplies before commit");
    require(gateway.commitConsumables(501),"committing reserved bandages failed");
    require(gateway.stockRemaining(MedicalSupply::Bandage)==1,"committed medical supplies were not consumed exactly once");
    gateway.release(501);
    require(!gateway.isExclusiveHeld(ResourceKind::Bed,8001),"release left stale bed reservation");
}

void testDeterministicPlanAndHospitalPipeline() {
    const auto& plan=standardHumanoidBodyPlan();
    auto a=makeTraumaFixture(1201);
    auto b=makeTraumaFixture(1201);
    require(planSignature(a)==planSignature(b),"identical wound fixtures produced different treatment plans");

    TestReservationGateway gateway;
    fillStock(gateway,128);
    std::vector<MedicalEvent> events;
    StableId nextJob=10000;
    int steps=0;
    while(steps<96) {
        const auto tasks=MedicalSystem::treatmentPlan(a,plan);
        if(tasks.empty()) break;
        const auto task=tasks.front();
        auto context=contextFor(nextJob++,task,9001);
        const auto result=MedicalSystem::executeTreatment(a,plan,task,context,gateway,&events);
        require(result.success,std::string("hospital pipeline blocked at ")+treatmentKindName(task.kind)+": "+result.reason);
        ++steps;
    }
    require(steps<96,"hospital pipeline failed to converge");
    require(a.medical.inMedicalZone,"rescue stage did not place patient in medical zone");
    require(a.medical.diagnosisComplete,"hospital pipeline never completed diagnosis");
    require(a.medical.stabilizationComplete,"hospital pipeline never stabilized patient");
    require(!a.diagnoses.empty() && !a.treatments.empty(),"medical record did not persist diagnosis/treatment entries");

    const auto rightHand=findPartByName(plan,"right_hand");
    const auto leftLeg=findPartByName(plan,"left_leg");
    require(rightHand && leftLeg,"body plan missing fixture parts after treatment");
    require(MedicalSystem::findPartState(a,*rightHand)->missing,"treatment incorrectly regenerated amputated tissue");
    require(std::any_of(a.prosthetics.begin(),a.prosthetics.end(),[&](const ProstheticRecord& p){
        return p.partId==*rightHand && p.installed && p.deviceStableId==9001;
    }),"prosthetic fitting did not create stable installed device state");
    require(MedicalSystem::findPartState(a,*leftLeg)->boneSet,"fracture never received bone care");
    require(a.capabilities.get(Capability::Grasp)>0.90f,"prosthetic did not restore selected grasp capability");

    require(std::any_of(events.begin(),events.end(),[](const MedicalEvent& e){return e.kind==MedicalEventKind::DiagnosisMade;}),
            "diagnosis event missing");
    require(std::any_of(events.begin(),events.end(),[](const MedicalEvent& e){return e.kind==MedicalEventKind::ProstheticInstalled && e.chronicleSignificant;}),
            "prosthetic history/significance event missing");
}

void testPersistentProstheticRoundTrip() {
    const auto& plan=standardHumanoidBodyPlan();
    auto patient=makeTraumaFixture(1301);
    TestReservationGateway gateway;
    fillStock(gateway,128);
    StableId nextJob=20000;
    for(int i=0;i<96;++i) {
        const auto tasks=MedicalSystem::treatmentPlan(patient,plan);
        if(tasks.empty()) break;
        const auto task=tasks.front();
        const auto result=MedicalSystem::executeTreatment(patient,plan,task,contextFor(nextJob++,task,99123),gateway);
        require(result.success,"round-trip fixture treatment failed");
    }
    require(!patient.prosthetics.empty(),"round-trip fixture did not reach prosthetic fitting");
    const float graspBefore=patient.capabilities.get(Capability::Grasp);
    const auto snapshot=MedicalSystem::capturePersistentState(patient);
    require(snapshot.schemaVersion==MedicalSnapshotSchemaVersion,"medical snapshot schema not versioned");
    require(snapshot.patientStableId==1301,"medical snapshot lost stable patient identity");

    auto restored=MedicalSystem::restorePersistentState(snapshot,plan);
    require(restored.patientStableId==patient.patientStableId,"restored patient stable identity changed");
    require(restored.prosthetics.size()==patient.prosthetics.size(),"prosthetic list did not survive unload/save/load seam");
    require(restored.prosthetics.front().deviceStableId==patient.prosthetics.front().deviceStableId,"prosthetic stable device ID changed on reload");
    require(std::abs(restored.capabilities.get(Capability::Grasp)-graspBefore)<0.0001f,
            "derived capability cache did not rebuild to same value after reload");
}

void testPhysiologyDeterminismAndDeath() {
    const auto& plan=standardHumanoidBodyPlan();
    auto a=MedicalSystem::makePatient(1401,plan);
    auto b=MedicalSystem::makePatient(1401,plan);
    const auto heart=findPartByName(plan,"heart");
    require(heart.has_value(),"body plan missing heart");
    InjurySpec wound{*heart,InjuryKind::Laceration,InjurySeverity::StructuralLoss,0.75f,0.20f,0.0f};
    MedicalSystem::applyInjury(a,plan,wound);
    MedicalSystem::applyInjury(b,plan,wound);
    for(int i=0;i<10;++i) {
        MedicalSystem::advancePhysiology(a,plan,30.0f);
        MedicalSystem::advancePhysiology(b,plan,30.0f);
    }
    require(std::abs(a.vitals.blood-b.vitals.blood)<1e-7f &&
            std::abs(a.vitals.shock-b.vitals.shock)<1e-7f &&
            a.medical.triage==b.medical.triage,
            "same wound/clock fixture produced nondeterministic physiology");

    // Untreated catastrophic bleeding eventually becomes a persistent death state.
    for(int i=0;i<30 && !a.medical.deceased;++i) MedicalSystem::advancePhysiology(a,plan,60.0f);
    require(a.medical.deceased,"untreated lethal bleeding never reached death state");
}

void testStageOrderingAndCanonicalRequirements() {
    const auto& plan=standardHumanoidBodyPlan();
    auto patient=makeTraumaFixture(1501);
    TestReservationGateway gateway;
    fillStock(gateway,32);

    const auto initial=MedicalSystem::treatmentPlan(patient,plan);
    const auto diagnoseIt=std::find_if(initial.begin(),initial.end(),[](const TreatmentTask& task){
        return task.kind==TreatmentKind::Diagnose;
    });
    require(diagnoseIt!=initial.end(),"trauma fixture has no diagnosis task");
    auto premature=contextFor(30001,*diagnoseIt);
    const auto skipped=MedicalSystem::executeTreatment(patient,plan,*diagnoseIt,premature,gateway);
    require(!skipped.success && skipped.reason.find("earlier treatment stage")!=std::string::npos,
            "medical job bypassed rescue/stage ordering");

    const auto rescue=initial.front();
    require(rescue.kind==TreatmentKind::RescueToHospital,"rescue is not first documented treatment stage");
    require(MedicalSystem::executeTreatment(patient,plan,rescue,contextFor(30002,rescue),gateway).success,
            "rescue stage failed in canonical requirement test");

    const auto afterRescue=MedicalSystem::treatmentPlan(patient,plan);
    auto diagnosis=*std::find_if(afterRescue.begin(),afterRescue.end(),[](const TreatmentTask& task){
        return task.kind==TreatmentKind::Diagnose;
    });
    // Forge the queued copy to claim no diagnosis skill and no bed. Execution
    // must re-read the canonical task instead of trusting caller-owned fields.
    diagnosis.skill=MedicalSkill::Rescue;
    diagnosis.minimumSkill=0.0f;
    diagnosis.needsBed=false;
    MedicalWorkerProfile unqualified;
    unqualified.stableId=7009;
    unqualified.skills.fill(0.0f);
    unqualified.skills[static_cast<std::size_t>(MedicalSkill::Rescue)]=1.0f;
    TreatmentExecutionContext forged;
    forged.jobStableId=30003;
    forged.worker=unqualified;
    forged.bedStableId=8001;
    forged.facilityStableId=8101;
    const auto rejected=MedicalSystem::executeTreatment(patient,plan,diagnosis,forged,gateway);
    require(!rejected.success && rejected.reason.find("required medical skill")!=std::string::npos,
            "forged treatment requirements bypassed canonical diagnosis skill gate");
}

void testProstheticMaintenanceAndService() {
    const auto& plan=standardHumanoidBodyPlan();
    auto patient=makeTraumaFixture(1601);
    TestReservationGateway gateway;
    fillStock(gateway,128);
    StableId nextJob=40000;
    for(int i=0;i<96;++i) {
        const auto tasks=MedicalSystem::treatmentPlan(patient,plan);
        if(tasks.empty()) break;
        const auto task=tasks.front();
        const auto result=MedicalSystem::executeTreatment(patient,plan,task,contextFor(nextJob++,task,96001),gateway);
        require(result.success,"maintenance fixture could not finish initial treatment");
    }
    require(patient.prosthetics.size()==1,"maintenance fixture did not install exactly one prosthetic");
    const float initialCondition=patient.prosthetics.front().condition;
    MedicalSystem::advancePhysiology(patient,plan,100.0f*3600.0f);
    require(patient.prosthetics.front().maintenance>=0.65f,"prosthetic did not accumulate service need over use time");
    require(patient.prosthetics.front().condition<initialCondition,"overdue prosthetic never degraded condition");

    const auto tasks=MedicalSystem::treatmentPlan(patient,plan);
    const auto serviceIt=std::find_if(tasks.begin(),tasks.end(),[](const TreatmentTask& task){
        return task.kind==TreatmentKind::ServiceProsthetic;
    });
    require(serviceIt!=tasks.end(),"overdue prosthetic did not generate a service job");
    const float beforeMaintenance=patient.prosthetics.front().maintenance;
    const float beforeCondition=patient.prosthetics.front().condition;
    std::vector<MedicalEvent> events;
    const auto result=MedicalSystem::executeTreatment(patient,plan,*serviceIt,contextFor(49999,*serviceIt,96001),gateway,&events);
    require(result.success,"prosthetic service job failed");
    require(patient.prosthetics.front().maintenance<beforeMaintenance,"prosthetic service did not reduce maintenance debt");
    require(patient.prosthetics.front().condition>beforeCondition,"prosthetic service did not restore device condition");
    require(std::any_of(events.begin(),events.end(),[](const MedicalEvent& e){return e.kind==MedicalEventKind::ProstheticServiced;}),
            "prosthetic service emitted no integration event");
}

void testInfectionHookEvent() {
    const auto& plan=standardHumanoidBodyPlan();
    auto patient=MedicalSystem::makePatient(1701,plan);
    const auto arm=findPartByName(plan,"left_arm");
    require(arm.has_value(),"body plan missing infection fixture arm");
    InjurySpec dirtyCut{*arm,InjuryKind::Laceration,InjurySeverity::Inhibited,0.20f,0.0f,0.80f};
    MedicalSystem::applyInjury(patient,plan,dirtyCut);
    const auto events=MedicalSystem::advancePhysiology(patient,plan,100.0f*60.0f);
    require(MedicalSystem::findWound(patient,1)->kind==InjuryKind::Infection,
            "untreated high-risk contaminated wound did not reach deterministic infection threshold");
    require(std::any_of(events.begin(),events.end(),[](const MedicalEvent& e){return e.kind==MedicalEventKind::SyndromeContracted;}),
            "infection transition emitted no syndrome/infection hook event");
}


void testBodyPlanAndSnapshotValidation() {
    const auto& plan=standardHumanoidBodyPlan();
    require(MedicalSystem::validateBodyPlan(plan).empty(),"standard body plan failed validation");

    BodyPlanDefinition broken=plan;
    broken.parts[1].id=broken.parts[0].id;
    require(!MedicalSystem::validateBodyPlan(broken).empty(),"duplicate body-part identity was not rejected");

    auto patient=MedicalSystem::makePatient(1801,plan);
    const auto hand=findPartByName(plan,"left_hand");
    require(hand.has_value(),"body plan missing validation fixture hand");
    MedicalSystem::applyInjury(patient,plan,{*hand,InjuryKind::Laceration,InjurySeverity::Inhibited,0.2f,0.0f,0.0f});
    auto state=MedicalSystem::capturePersistentState(patient);
    require(MedicalSystem::validatePersistentState(state,plan).empty(),"valid medical snapshot failed validation");

    auto malformed=state;
    malformed.nextWoundId=1;
    require(!MedicalSystem::validatePersistentState(malformed,plan).empty(),"colliding next wound identity was not rejected");
    bool threw=false;
    try { (void)MedicalSystem::restorePersistentState(malformed,plan); }
    catch(const std::runtime_error&) { threw=true; }
    require(threw,"restore accepted malformed medical snapshot");
}

void testMedicalInspectorExplainability() {
    const auto& plan=standardHumanoidBodyPlan();
    auto patient=MedicalSystem::makePatient(1802,plan);
    const auto leg=findPartByName(plan,"right_leg");
    require(leg.has_value(),"body plan missing inspector fixture leg");
    MedicalSystem::applyInjury(patient,plan,{*leg,InjuryKind::Laceration,InjurySeverity::FunctionLoss,0.45f,-1.0f,0.35f});
    const auto report=MedicalSystem::inspect(patient,plan);
    require(report.activeWoundCount==1,"inspector active wound count incorrect");
    require(report.uncontrolledBleeding,"inspector failed to report uncontrolled bleeding");
    require(report.unmanagedContamination,"inspector failed to report unmanaged contamination");
    require(report.nextTreatment.has_value(),"inspector failed to expose next treatment");
    require(!report.reasons.empty(),"inspector did not explain blocked readiness");
}

void testPoweredProstheticCapabilityTransition() {
    const auto& plan=standardHumanoidBodyPlan();
    auto patient=MedicalSystem::makePatient(1803,plan);
    const auto hand=findPartByName(plan,"right_hand");
    require(hand.has_value(),"body plan missing powered prosthetic fixture hand");
    MedicalSystem::applyInjury(patient,plan,{*hand,InjuryKind::Amputation,InjurySeverity::Missing,1.0f,0.0f,0.0f});

    ProstheticRecord device;
    device.deviceStableId=99001;
    device.partId=*hand;
    device.kind=ProstheticKind::CyberneticLimb;
    device.requiresPower=true;
    device.powered=true;
    device.installed=true;
    device.restoration[static_cast<std::size_t>(Capability::Grasp)]=0.72f;
    device.restoration[static_cast<std::size_t>(Capability::FineWork)]=0.60f;
    patient.prosthetics.push_back(device);
    MedicalSystem::rebuildCapabilities(patient,plan);
    const float poweredGrasp=patient.capabilities.get(Capability::Grasp);

    std::vector<MedicalEvent> events;
    require(MedicalSystem::setProstheticPowered(patient,plan,99001,false,&events),"powered prosthetic state transition failed");
    const float unpoweredGrasp=patient.capabilities.get(Capability::Grasp);
    require(unpoweredGrasp<poweredGrasp,"unpowered cybernetic prosthetic retained powered capability");
    require(std::any_of(events.begin(),events.end(),[](const MedicalEvent& e){ return e.kind==MedicalEventKind::CapabilityChanged; }),
            "prosthetic power transition emitted no capability event");
    require(MedicalSystem::setProstheticPowered(patient,plan,99001,true,nullptr),"prosthetic repower failed");
    require(patient.capabilities.get(Capability::Grasp)>unpoweredGrasp,"repowered prosthetic did not restore capability");
}

} // namespace

int main() {
    try {
        testBodyPlanAndCapabilityLoss();
        testDirectActionDamageBridge();
        testReservationAtomicityAndNoDoubleBook();
        testDeterministicPlanAndHospitalPipeline();
        testPersistentProstheticRoundTrip();
        testPhysiologyDeterminismAndDeath();
        testStageOrderingAndCanonicalRequirements();
        testProstheticMaintenanceAndService();
        testInfectionHookEvent();
        testBodyPlanAndSnapshotValidation();
        testMedicalInspectorExplainability();
        testPoweredProstheticCapabilityTransition();
        std::cout << "medical tests passed\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "medical test failure: " << e.what() << '\n';
        return 1;
    }
}
