// Intended function: imported medical implementation for MedicalSystem; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace elysium::medical {

// Medical identities are durable game identities, deliberately independent of
// entt::entity and any process-local registry slot. Agent 19/21 adapters can
// therefore reserve the same patients/resources across ECS shard promotion.
using StableId = std::uint64_t;
using BodyPartId = std::uint16_t;
using WoundId = std::uint64_t;

constexpr StableId InvalidStableId = 0;
constexpr BodyPartId InvalidBodyPart = 0xffffu;
constexpr std::uint32_t MedicalSnapshotSchemaVersion = 1;

enum class BodyPartRole : std::uint8_t {
    Torso,
    Head,
    Brain,
    Heart,
    Lung,
    Arm,
    Hand,
    Leg,
    Foot,
    Eye,
    Ear,
    Mouth,
    Other
};

enum class Capability : std::uint8_t {
    Walk,
    Grasp,
    See,
    Hear,
    Breathe,
    Speak,
    FineWork,
    HeavyWork,
    Combat,
    Conscious,
    Count
};

constexpr std::size_t CapabilityCount = static_cast<std::size_t>(Capability::Count);
using CapabilityVector = std::array<float, CapabilityCount>;

enum class InjuryKind : std::uint8_t {
    Laceration,
    Burn,
    Fracture,
    Crush,
    OrganDamage,
    Corrosion,
    Irradiation,
    Contamination,
    Infection,
    Syndrome,
    Amputation
};

enum class InjurySeverity : std::uint8_t {
    Minor,
    Inhibited,
    FunctionLoss,
    StructuralLoss,
    Missing,
    Systemic
};

enum class TriageCategory : std::uint8_t {
    Routine,
    Delayed,
    Urgent,
    Immediate,
    Expectant,
    Deceased
};

enum class TreatmentStage : std::uint8_t {
    Rescue,
    Diagnose,
    Stabilize,
    Surgery,
    BoneCare,
    WoundCare,
    Medication,
    Recovery,
    Rehabilitation,
    Complete
};

enum class TreatmentKind : std::uint8_t {
    RescueToHospital,
    Diagnose,
    StopBleeding,
    RestoreAirwayOxygen,
    Decontaminate,
    TreatShock,
    Surgery,
    BoneSetting,
    Suture,
    DressWound,
    AdministerMedication,
    MonitorRecovery,
    Rehabilitation,
    FitProsthetic,
    ServiceProsthetic
};

enum class MedicalSkill : std::uint8_t {
    Rescue,
    Diagnosis,
    TraumaCare,
    Surgery,
    BoneSetting,
    WoundCare,
    Pharmacology,
    Rehabilitation,
    Prosthetics,
    Count
};

constexpr std::size_t MedicalSkillCount = static_cast<std::size_t>(MedicalSkill::Count);

enum class ResourceKind : std::uint8_t {
    Worker,
    Bed,
    Facility,
    Tool,
    Supply,
    Device
};

enum class MedicalSupply : std::uint16_t {
    Bandage = 1,
    Oxygen = 2,
    DeconAgent = 3,
    SurgicalKit = 4,
    Splint = 5,
    Suture = 6,
    Dressing = 7,
    Medication = 8,
    Food = 9,
    ProstheticParts = 10
};

enum class ProstheticKind : std::uint8_t {
    Crutch,
    Brace,
    MechanicalLimb,
    CyberneticLimb,
    SensoryAid,
    ImplantedFilter,
    PoweredExoskeleton
};

enum class MedicalEventKind : std::uint8_t {
    CitizenWounded,
    DiagnosisMade,
    TreatmentPerformed,
    MajorDisability,
    ScarFormed,
    ProstheticInstalled,
    ProstheticServiced,
    SyndromeContracted,
    CapabilityChanged,
    CitizenDied
};

struct BodyPartDefinition {
    BodyPartId id{InvalidBodyPart};
    BodyPartId parent{InvalidBodyPart};
    BodyPartRole role{BodyPartRole::Other};
    std::string name;
    CapabilityVector contribution{};
    float vitalWeight{};       // 0 = non-vital, larger = stronger systemic consequence.
    float bleedingScale{1.0f};
    bool paired{};
};

struct BodyPlanDefinition {
    std::string contentId;     // namespaced stable content identifier.
    std::uint32_t version{1};
    std::vector<BodyPartDefinition> parts;
};

// Immutable standard body plan used by the current prototype. Species systems
// can supply other plans later without changing patient state semantics.
const BodyPlanDefinition& standardHumanoidBodyPlan();
const BodyPartDefinition* findPart(const BodyPlanDefinition& plan, BodyPartId id);
std::optional<BodyPartId> findPartByName(const BodyPlanDefinition& plan, std::string_view name);

struct BodyPartState {
    BodyPartId partId{InvalidBodyPart};
    float condition{1.0f};          // 1 healthy, 0 destroyed.
    float nerveFunction{1.0f};      // used for functional loss independent of tissue integrity.
    float contamination{};
    float fracture{};               // 0 none, 1 maximally unstable.
    bool missing{};
    bool surgicallyStabilized{};
    bool boneSet{};
};

struct BodyVitals {
    float blood{1.0f};              // normalized systemic proxy.
    float oxygen{1.0f};
    float pain{};
    float consciousness{1.0f};
    float shock{};
    float temperature{1.0f};        // normalized healthy baseline; not degrees.
    float systemicToxin{};
};

struct WoundRecord {
    WoundId id{};
    BodyPartId partId{InvalidBodyPart};
    InjuryKind kind{InjuryKind::Laceration};
    InjurySeverity severity{InjurySeverity::Minor};
    float tissueDamage{};
    float bleedPerMinute{};
    float pain{};
    float contamination{};
    float infectionRisk{};
    float healing{};                // 0 newly wounded, 1 healed.
    bool diagnosed{};
    bool stabilized{};             // derived immediate-stabilization summary for this wound.
    bool bleedingControlled{};
    bool decontaminated{};
    bool surgeryRequired{};
    bool surgeryComplete{};
    bool boneCareRequired{};
    bool boneSet{};
    bool closureRequired{};
    bool sutured{};
    bool dressed{};
    bool medicationRequired{};
    bool medicated{};
    bool scarRecorded{};
};

struct ScarRecord {
    WoundId sourceWound{};
    BodyPartId partId{InvalidBodyPart};
    InjuryKind origin{InjuryKind::Laceration};
    float severity{};
};

struct ProstheticRecord {
    StableId deviceStableId{};
    BodyPartId partId{InvalidBodyPart};
    ProstheticKind kind{ProstheticKind::MechanicalLimb};
    CapabilityVector restoration{};
    float condition{1.0f};
    float maintenance{};            // 0 serviced -> 1 service overdue.
    bool requiresPower{};
    bool powered{true};
    bool installed{};
};

struct DiagnosisRecord {
    WoundId woundId{};
    InjuryKind kind{InjuryKind::Laceration};
    InjurySeverity severity{InjurySeverity::Minor};
    std::uint32_t sequence{};
};

struct TreatmentRecord {
    TreatmentKind kind{TreatmentKind::Diagnose};
    WoundId woundId{};
    StableId workerStableId{};
    std::uint32_t sequence{};
    float quality{};
};

struct MedicalStatus {
    TriageCategory triage{TriageCategory::Routine};
    StableId hospitalStableId{};
    StableId bedStableId{};
    bool inMedicalZone{};
    bool diagnosisComplete{};
    bool stabilizationComplete{};
    bool medicallyStable{};
    bool readyForWork{true};
    bool deceased{};
    float recoveryProgress{1.0f};
    std::string blockedReason;
};

struct CapabilitySummary {
    CapabilityVector values{};
    std::uint32_t revision{};

    float get(Capability capability) const {
        return values[static_cast<std::size_t>(capability)];
    }
};

struct PatientMedicalState {
    StableId patientStableId{};
    std::string bodyPlanContentId{"elysium:body/humanoid_standard_v1"};
    BodyVitals vitals{};
    std::vector<BodyPartState> bodyParts;
    std::vector<WoundRecord> wounds;
    std::vector<ScarRecord> scars;
    std::vector<ProstheticRecord> prosthetics;
    std::vector<DiagnosisRecord> diagnoses;
    std::vector<TreatmentRecord> treatments;
    MedicalStatus medical{};
    CapabilitySummary capabilities{}; // derived/cache, rebuilt after load.
    WoundId nextWoundId{1};
    std::uint32_t nextRecordSequence{1};
};

struct InjurySpec {
    BodyPartId partId{InvalidBodyPart};
    InjuryKind kind{InjuryKind::Laceration};
    InjurySeverity severity{InjurySeverity::Minor};
    float tissueDamage{};           // 0 lets system derive from severity.
    float bleedPerMinute{};         // <0 lets system derive; zero is explicit no bleeding.
    float contamination{};
};

struct DirectDamageSpec {
    float scalarDamage{};           // intended 0..100 style action-RPG damage.
    InjuryKind kind{InjuryKind::Laceration};
    BodyPartId partId{InvalidBodyPart};
};

struct TreatmentTask {
    TreatmentStage stage{TreatmentStage::Complete};
    TreatmentKind kind{TreatmentKind::MonitorRecovery};
    WoundId woundId{};              // 0 means systemic/global task.
    BodyPartId partId{InvalidBodyPart};
    MedicalSkill skill{MedicalSkill::TraumaCare};
    float minimumSkill{};           // 0..1 normalized proficiency.
    bool needsBed{};
    bool needsFacility{};
    std::vector<std::pair<MedicalSupply, int>> supplies;
};

struct MedicalWorkerProfile {
    StableId stableId{};
    std::array<float, MedicalSkillCount> skills{};

    float skill(MedicalSkill which) const {
        return skills[static_cast<std::size_t>(which)];
    }
};

struct ResourceRequest {
    ResourceKind kind{ResourceKind::Worker};
    StableId stableId{};            // worker/bed/facility/tool/device identity.
    std::uint32_t contentId{};      // supply/content identity where stable object ID is not used.
    int quantity{1};
    bool exclusive{true};
};

struct ReservationAttempt {
    bool success{};
    std::string reason;
};

// Adapter boundary for Agent 19/21 jobs/stockpiles. The medical subsystem does
// not own the global job scheduler or inventory graph. The implementation must
// atomically reserve the whole request set or reserve none of it.
class IMedicalReservationGateway {
public:
    virtual ~IMedicalReservationGateway() = default;
    virtual ReservationAttempt tryReserveAll(StableId jobStableId,
                                             std::span<const ResourceRequest> requests) = 0;
    virtual bool commitConsumables(StableId jobStableId) = 0;
    virtual void release(StableId jobStableId) = 0;
};

struct TreatmentExecutionContext {
    StableId jobStableId{};
    MedicalWorkerProfile worker{};
    StableId bedStableId{};
    StableId facilityStableId{};
    StableId deviceStableId{};      // used by prosthetic fitting.
    ProstheticKind prostheticKind{ProstheticKind::MechanicalLimb};
};

struct TreatmentResult {
    bool success{};
    std::string reason;
    float quality{};
    std::vector<ResourceRequest> reservations;
};

struct MedicalEvent {
    MedicalEventKind kind{MedicalEventKind::CitizenWounded};
    StableId patientStableId{};
    WoundId woundId{};
    BodyPartId partId{InvalidBodyPart};
    TreatmentKind treatment{TreatmentKind::Diagnose};
    float magnitude{};
    bool chronicleSignificant{};
};

struct MedicalInspectionReport {
    TriageCategory triage{TriageCategory::Routine};
    bool readyForWork{};
    bool medicallyStable{};
    bool uncontrolledBleeding{};
    bool unmanagedContamination{};
    bool infectionOrSyndrome{};
    bool prostheticServiceDue{};
    std::size_t activeWoundCount{};
    CapabilityVector capabilities{};
    std::optional<TreatmentTask> nextTreatment;
    std::vector<std::string> reasons;
};

struct MedicalPersistentState {
    std::uint32_t schemaVersion{MedicalSnapshotSchemaVersion};
    StableId patientStableId{};
    std::string bodyPlanContentId;
    BodyVitals vitals{};
    std::vector<BodyPartState> bodyParts;
    std::vector<WoundRecord> wounds;
    std::vector<ScarRecord> scars;
    std::vector<ProstheticRecord> prosthetics;
    std::vector<DiagnosisRecord> diagnoses;
    std::vector<TreatmentRecord> treatments;
    MedicalStatus medical{};
    WoundId nextWoundId{1};
    std::uint32_t nextRecordSequence{1};
};

class MedicalSystem {
public:
    static PatientMedicalState makePatient(StableId patientStableId,
                                           const BodyPlanDefinition& plan = standardHumanoidBodyPlan());

    static std::vector<MedicalEvent> applyInjury(PatientMedicalState& patient,
                                                 const BodyPlanDefinition& plan,
                                                 const InjurySpec& spec);
    static std::vector<MedicalEvent> applyDirectDamage(PatientMedicalState& patient,
                                                       const BodyPlanDefinition& plan,
                                                       const DirectDamageSpec& damage);

    static void rebuildCapabilities(PatientMedicalState& patient,
                                    const BodyPlanDefinition& plan);
    static TriageCategory evaluateTriage(const PatientMedicalState& patient);
    static std::vector<TreatmentTask> treatmentPlan(const PatientMedicalState& patient,
                                                    const BodyPlanDefinition& plan);

    static TreatmentResult executeTreatment(PatientMedicalState& patient,
                                            const BodyPlanDefinition& plan,
                                            const TreatmentTask& task,
                                            const TreatmentExecutionContext& context,
                                            IMedicalReservationGateway& reservations,
                                            std::vector<MedicalEvent>* emittedEvents = nullptr);

    static std::vector<MedicalEvent> advancePhysiology(PatientMedicalState& patient,
                                                       const BodyPlanDefinition& plan,
                                                       float elapsedSeconds);

    static float scalarHealthEquivalent(const PatientMedicalState& patient,
                                        const BodyPlanDefinition& plan,
                                        float maxHealth = 100.0f);

    static MedicalPersistentState capturePersistentState(const PatientMedicalState& patient);
    static std::vector<std::string> validateBodyPlan(const BodyPlanDefinition& plan);
    static std::vector<std::string> validatePersistentState(const MedicalPersistentState& state,
                                                            const BodyPlanDefinition& plan = standardHumanoidBodyPlan());
    static MedicalInspectionReport inspect(const PatientMedicalState& patient,
                                           const BodyPlanDefinition& plan = standardHumanoidBodyPlan());
    static bool setProstheticPowered(PatientMedicalState& patient,
                                     const BodyPlanDefinition& plan,
                                     StableId deviceStableId,
                                     bool powered,
                                     std::vector<MedicalEvent>* emittedEvents = nullptr);
    static PatientMedicalState restorePersistentState(const MedicalPersistentState& state,
                                                       const BodyPlanDefinition& plan = standardHumanoidBodyPlan());

    static const WoundRecord* findWound(const PatientMedicalState& patient, WoundId id);
    static WoundRecord* findWound(PatientMedicalState& patient, WoundId id);
    static const BodyPartState* findPartState(const PatientMedicalState& patient, BodyPartId id);
    static BodyPartState* findPartState(PatientMedicalState& patient, BodyPartId id);
};

const char* treatmentKindName(TreatmentKind kind);
const char* triageName(TriageCategory category);

} // namespace elysium::medical
