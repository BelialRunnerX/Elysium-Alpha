// Intended function: imported medical implementation for MedicalSystem; preserves the agent-authored subsystem contract for later integration/debugging.
#include "medical/MedicalSystem.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <unordered_set>

namespace elysium::medical {
namespace {

constexpr float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
constexpr std::size_t idx(Capability c) { return static_cast<std::size_t>(c); }
constexpr std::size_t skillIdx(MedicalSkill s) { return static_cast<std::size_t>(s); }

CapabilityVector contribution(std::initializer_list<std::pair<Capability, float>> values) {
    CapabilityVector out{};
    for (const auto& [c, v] : values) out[idx(c)] = v;
    return out;
}

const BodyPlanDefinition& buildStandardHumanoid() {
    static const BodyPlanDefinition plan = [] {
        BodyPlanDefinition p;
        p.contentId = "elysium:body/humanoid_standard_v1";
        p.version = 1;
        // Explicit IDs are persistence protocol values for this content definition.
        p.parts = {
            { 1, InvalidBodyPart, BodyPartRole::Torso, "torso",
              contribution({{Capability::Breathe,0.05f},{Capability::HeavyWork,0.15f},{Capability::Combat,0.10f},{Capability::Conscious,0.05f}}), 0.55f, 1.0f, false},
            { 2, 1, BodyPartRole::Head, "head",
              contribution({{Capability::Speak,0.20f},{Capability::Conscious,0.15f}}), 0.35f, 1.2f, false},
            { 3, 2, BodyPartRole::Brain, "brain",
              contribution({{Capability::Conscious,0.80f},{Capability::FineWork,0.10f},{Capability::Combat,0.10f}}), 1.00f, 0.25f, false},
            { 4, 1, BodyPartRole::Heart, "heart",
              contribution({{Capability::Conscious,0.10f},{Capability::HeavyWork,0.10f},{Capability::Combat,0.05f}}), 1.00f, 2.0f, false},
            { 5, 1, BodyPartRole::Lung, "left_lung",
              contribution({{Capability::Breathe,0.50f},{Capability::HeavyWork,0.05f}}), 0.75f, 1.4f, true},
            { 6, 1, BodyPartRole::Lung, "right_lung",
              contribution({{Capability::Breathe,0.50f},{Capability::HeavyWork,0.05f}}), 0.75f, 1.4f, true},
            { 7, 1, BodyPartRole::Arm, "left_arm",
              contribution({{Capability::Grasp,0.10f},{Capability::FineWork,0.10f},{Capability::HeavyWork,0.20f},{Capability::Combat,0.10f}}), 0.0f, 1.1f, true},
            { 8, 1, BodyPartRole::Arm, "right_arm",
              contribution({{Capability::Grasp,0.10f},{Capability::FineWork,0.10f},{Capability::HeavyWork,0.20f},{Capability::Combat,0.10f}}), 0.0f, 1.1f, true},
            { 9, 7, BodyPartRole::Hand, "left_hand",
              contribution({{Capability::Grasp,0.40f},{Capability::FineWork,0.40f},{Capability::Combat,0.15f}}), 0.0f, 1.0f, true},
            {10, 8, BodyPartRole::Hand, "right_hand",
              contribution({{Capability::Grasp,0.40f},{Capability::FineWork,0.40f},{Capability::Combat,0.15f}}), 0.0f, 1.0f, true},
            {11, 1, BodyPartRole::Leg, "left_leg",
              contribution({{Capability::Walk,0.40f},{Capability::HeavyWork,0.10f},{Capability::Combat,0.10f}}), 0.0f, 1.4f, true},
            {12, 1, BodyPartRole::Leg, "right_leg",
              contribution({{Capability::Walk,0.40f},{Capability::HeavyWork,0.10f},{Capability::Combat,0.10f}}), 0.0f, 1.4f, true},
            {13,11, BodyPartRole::Foot, "left_foot",
              contribution({{Capability::Walk,0.10f},{Capability::Combat,0.025f}}), 0.0f, 0.8f, true},
            {14,12, BodyPartRole::Foot, "right_foot",
              contribution({{Capability::Walk,0.10f},{Capability::Combat,0.025f}}), 0.0f, 0.8f, true},
            {15, 2, BodyPartRole::Eye, "left_eye",
              contribution({{Capability::See,0.50f},{Capability::Combat,0.025f}}), 0.0f, 0.4f, true},
            {16, 2, BodyPartRole::Eye, "right_eye",
              contribution({{Capability::See,0.50f},{Capability::Combat,0.025f}}), 0.0f, 0.4f, true},
            {17, 2, BodyPartRole::Ear, "left_ear",
              contribution({{Capability::Hear,0.50f}}), 0.0f, 0.5f, true},
            {18, 2, BodyPartRole::Ear, "right_ear",
              contribution({{Capability::Hear,0.50f}}), 0.0f, 0.5f, true},
            {19, 2, BodyPartRole::Mouth, "mouth",
              contribution({{Capability::Speak,0.80f}}), 0.0f, 0.8f, false}
        };
        return p;
    }();
    return plan;
}

float severityDamage(InjurySeverity severity) {
    switch (severity) {
        case InjurySeverity::Minor: return 0.08f;
        case InjurySeverity::Inhibited: return 0.22f;
        case InjurySeverity::FunctionLoss: return 0.48f;
        case InjurySeverity::StructuralLoss: return 0.76f;
        case InjurySeverity::Missing: return 1.0f;
        case InjurySeverity::Systemic: return 0.38f;
    }
    return 0.1f;
}

float severityPain(InjurySeverity severity) {
    switch (severity) {
        case InjurySeverity::Minor: return 0.10f;
        case InjurySeverity::Inhibited: return 0.28f;
        case InjurySeverity::FunctionLoss: return 0.52f;
        case InjurySeverity::StructuralLoss: return 0.82f;
        case InjurySeverity::Missing: return 0.92f;
        case InjurySeverity::Systemic: return 0.58f;
    }
    return 0.1f;
}

float defaultBleed(const BodyPartDefinition& part, InjuryKind kind, InjurySeverity severity) {
    float base = 0.0f;
    switch (kind) {
        case InjuryKind::Laceration: base = 0.025f; break;
        case InjuryKind::Crush: base = 0.018f; break;
        case InjuryKind::OrganDamage: base = 0.035f; break;
        case InjuryKind::Amputation: base = 0.060f; break;
        case InjuryKind::Corrosion: base = 0.010f; break;
        case InjuryKind::Burn:
        case InjuryKind::Fracture:
        case InjuryKind::Irradiation:
        case InjuryKind::Contamination:
        case InjuryKind::Infection:
        case InjuryKind::Syndrome: base = 0.0f; break;
    }
    const float sev = 0.5f + severityDamage(severity) * 1.5f;
    return base * sev * part.bleedingScale;
}

bool isOpenWound(InjuryKind kind) {
    return kind == InjuryKind::Laceration || kind == InjuryKind::Crush ||
           kind == InjuryKind::OrganDamage || kind == InjuryKind::Amputation ||
           kind == InjuryKind::Corrosion;
}

bool needsSurgery(InjuryKind kind, InjurySeverity severity) {
    if (kind == InjuryKind::OrganDamage || kind == InjuryKind::Crush || kind == InjuryKind::Amputation)
        return severity >= InjurySeverity::FunctionLoss;
    return severity >= InjurySeverity::StructuralLoss && kind != InjuryKind::Fracture;
}

bool requiresMedication(InjuryKind kind, float contamination) {
    return kind == InjuryKind::Infection || kind == InjuryKind::Syndrome ||
           kind == InjuryKind::Irradiation || contamination > 0.15f;
}

bool isHealed(const WoundRecord& wound) { return wound.healing >= 0.999f; }

int triageRank(TriageCategory category) {
    switch (category) {
        case TriageCategory::Routine: return 0;
        case TriageCategory::Delayed: return 1;
        case TriageCategory::Urgent: return 2;
        case TriageCategory::Immediate: return 3;
        case TriageCategory::Expectant: return 4;
        case TriageCategory::Deceased: return 5;
    }
    return 0;
}

TreatmentStage stageFor(TreatmentKind kind) {
    switch (kind) {
        case TreatmentKind::RescueToHospital: return TreatmentStage::Rescue;
        case TreatmentKind::Diagnose: return TreatmentStage::Diagnose;
        case TreatmentKind::StopBleeding:
        case TreatmentKind::RestoreAirwayOxygen:
        case TreatmentKind::Decontaminate:
        case TreatmentKind::TreatShock: return TreatmentStage::Stabilize;
        case TreatmentKind::Surgery: return TreatmentStage::Surgery;
        case TreatmentKind::BoneSetting: return TreatmentStage::BoneCare;
        case TreatmentKind::Suture:
        case TreatmentKind::DressWound: return TreatmentStage::WoundCare;
        case TreatmentKind::AdministerMedication: return TreatmentStage::Medication;
        case TreatmentKind::MonitorRecovery: return TreatmentStage::Recovery;
        case TreatmentKind::Rehabilitation:
        case TreatmentKind::FitProsthetic:
        case TreatmentKind::ServiceProsthetic: return TreatmentStage::Rehabilitation;
    }
    return TreatmentStage::Complete;
}

MedicalSkill skillFor(TreatmentKind kind) {
    switch (kind) {
        case TreatmentKind::RescueToHospital: return MedicalSkill::Rescue;
        case TreatmentKind::Diagnose: return MedicalSkill::Diagnosis;
        case TreatmentKind::StopBleeding:
        case TreatmentKind::RestoreAirwayOxygen:
        case TreatmentKind::Decontaminate:
        case TreatmentKind::TreatShock: return MedicalSkill::TraumaCare;
        case TreatmentKind::Surgery: return MedicalSkill::Surgery;
        case TreatmentKind::BoneSetting: return MedicalSkill::BoneSetting;
        case TreatmentKind::Suture:
        case TreatmentKind::DressWound: return MedicalSkill::WoundCare;
        case TreatmentKind::AdministerMedication: return MedicalSkill::Pharmacology;
        case TreatmentKind::MonitorRecovery:
        case TreatmentKind::Rehabilitation: return MedicalSkill::Rehabilitation;
        case TreatmentKind::FitProsthetic:
        case TreatmentKind::ServiceProsthetic: return MedicalSkill::Prosthetics;
    }
    return MedicalSkill::TraumaCare;
}

float minimumSkillFor(TreatmentKind kind) {
    switch (kind) {
        case TreatmentKind::RescueToHospital: return 0.05f;
        case TreatmentKind::Diagnose: return 0.20f;
        case TreatmentKind::StopBleeding: return 0.18f;
        case TreatmentKind::RestoreAirwayOxygen: return 0.20f;
        case TreatmentKind::Decontaminate: return 0.15f;
        case TreatmentKind::TreatShock: return 0.30f;
        case TreatmentKind::Surgery: return 0.55f;
        case TreatmentKind::BoneSetting: return 0.35f;
        case TreatmentKind::Suture: return 0.30f;
        case TreatmentKind::DressWound: return 0.15f;
        case TreatmentKind::AdministerMedication: return 0.25f;
        case TreatmentKind::MonitorRecovery: return 0.10f;
        case TreatmentKind::Rehabilitation: return 0.25f;
        case TreatmentKind::FitProsthetic: return 0.45f;
        case TreatmentKind::ServiceProsthetic: return 0.30f;
    }
    return 0.2f;
}

std::vector<std::pair<MedicalSupply,int>> suppliesFor(const TreatmentTask& task) {
    switch (task.kind) {
        case TreatmentKind::StopBleeding: return {{MedicalSupply::Bandage,1}};
        case TreatmentKind::RestoreAirwayOxygen: return {{MedicalSupply::Oxygen,1}};
        case TreatmentKind::Decontaminate: return {{MedicalSupply::DeconAgent,1}};
        case TreatmentKind::Surgery: return {{MedicalSupply::SurgicalKit,1},{MedicalSupply::Bandage,1}};
        case TreatmentKind::BoneSetting: return {{MedicalSupply::Splint,1}};
        case TreatmentKind::Suture: return {{MedicalSupply::Suture,1}};
        case TreatmentKind::DressWound: return {{MedicalSupply::Dressing,1}};
        case TreatmentKind::AdministerMedication: return {{MedicalSupply::Medication,1}};
        case TreatmentKind::MonitorRecovery: return {{MedicalSupply::Food,1}};
        case TreatmentKind::FitProsthetic: return {{MedicalSupply::ProstheticParts,1}};
        case TreatmentKind::ServiceProsthetic: return {{MedicalSupply::ProstheticParts,1}};
        default: return {};
    }
}

TreatmentTask makeTask(TreatmentKind kind, WoundId woundId = 0, BodyPartId partId = InvalidBodyPart) {
    TreatmentTask task;
    task.stage = stageFor(kind);
    task.kind = kind;
    task.woundId = woundId;
    task.partId = partId;
    task.skill = skillFor(kind);
    task.minimumSkill = minimumSkillFor(kind);
    task.needsBed = kind != TreatmentKind::RescueToHospital && kind != TreatmentKind::Rehabilitation;
    task.needsFacility = kind == TreatmentKind::Surgery || kind == TreatmentKind::BoneSetting ||
                         kind == TreatmentKind::FitProsthetic || kind == TreatmentKind::ServiceProsthetic;
    task.supplies = suppliesFor(task);
    return task;
}

bool anyUnstabilizedBleed(const PatientMedicalState& p) {
    return std::any_of(p.wounds.begin(), p.wounds.end(), [](const WoundRecord& w) {
        return !isHealed(w) && w.bleedPerMinute > 0.0001f && !w.bleedingControlled;
    });
}

float totalBleed(const PatientMedicalState& p) {
    float sum = 0.0f;
    for (const auto& w : p.wounds)
        if (!isHealed(w) && !w.bleedingControlled) sum += std::max(0.0f, w.bleedPerMinute) * (1.0f - w.healing);
    return sum;
}

bool anyUnmanagedContamination(const PatientMedicalState& p) {
    for (const auto& w : p.wounds)
        if (!isHealed(w) && w.contamination > 0.12f && !w.decontaminated) return true;
    for (const auto& part : p.bodyParts)
        if (part.contamination > 0.12f) return true;
    return false;
}

float naturalCapability(const PatientMedicalState& patient,
                       const BodyPlanDefinition& plan,
                       Capability capability) {
    float totalWeight = 0.0f;
    float available = 0.0f;
    for (const auto& def : plan.parts) {
        const float weight = def.contribution[idx(capability)];
        if (weight <= 0.0f) continue;
        totalWeight += weight;
        const auto* state = MedicalSystem::findPartState(patient, def.id);
        if (!state || state->missing) continue;
        const float function = clamp01(state->condition) * clamp01(state->nerveFunction);
        available += weight * function;
    }
    if (totalWeight <= 0.0001f) return 1.0f;
    return clamp01(available / totalWeight);
}

float prostheticFloor(const PatientMedicalState& patient, Capability capability) {
    float restore = 0.0f;
    for (const auto& device : patient.prosthetics) {
        if (!device.installed || device.condition <= 0.05f) continue;
        if (device.requiresPower && !device.powered) continue;
        restore += device.restoration[idx(capability)] * clamp01(device.condition) * (1.0f - 0.35f * clamp01(device.maintenance));
    }
    return clamp01(restore);
}

void addEvent(std::vector<MedicalEvent>* events, MedicalEvent event) {
    if (events) events->push_back(event);
}

InjurySeverity severityFromDirectDamage(float damage) {
    if (damage < 8.0f) return InjurySeverity::Minor;
    if (damage < 18.0f) return InjurySeverity::Inhibited;
    if (damage < 36.0f) return InjurySeverity::FunctionLoss;
    if (damage < 65.0f) return InjurySeverity::StructuralLoss;
    return InjurySeverity::Missing;
}

void sortPlan(std::vector<TreatmentTask>& tasks) {
    std::stable_sort(tasks.begin(), tasks.end(), [](const TreatmentTask& a, const TreatmentTask& b) {
        return std::tuple{static_cast<int>(a.stage), a.woundId, static_cast<int>(a.kind), a.partId} <
               std::tuple{static_cast<int>(b.stage), b.woundId, static_cast<int>(b.kind), b.partId};
    });
}

WoundRecord* taskWound(PatientMedicalState& patient, const TreatmentTask& task) {
    return task.woundId == 0 ? nullptr : MedicalSystem::findWound(patient, task.woundId);
}

bool allRequiredCareComplete(const PatientMedicalState& p) {
    for (const auto& w : p.wounds) {
        if (isHealed(w)) continue;
        if (!w.diagnosed) return false;
        if (w.bleedPerMinute > 0.0001f && !w.bleedingControlled) return false;
        if (w.contamination > 0.12f && !w.decontaminated) return false;
        if (w.surgeryRequired && !w.surgeryComplete) return false;
        if (w.boneCareRequired && !w.boneSet) return false;
        if (w.closureRequired && !(w.sutured || w.dressed)) return false;
        if (w.medicationRequired && !w.medicated) return false;
    }
    return true;
}

void updateStatus(PatientMedicalState& p) {
    p.medical.triage = MedicalSystem::evaluateTriage(p);
    p.medical.deceased = p.medical.triage == TriageCategory::Deceased;
    p.medical.stabilizationComplete = !anyUnstabilizedBleed(p) && p.vitals.shock < 0.40f && !anyUnmanagedContamination(p);
    p.medical.medicallyStable = !p.medical.deceased && p.medical.stabilizationComplete && p.vitals.blood > 0.45f && p.vitals.oxygen > 0.55f;
    p.medical.readyForWork = p.medical.medicallyStable && p.capabilities.get(Capability::Conscious) > 0.70f &&
                             p.capabilities.get(Capability::Walk) > 0.35f && p.medical.recoveryProgress > 0.65f;
}

CapabilityVector defaultProstheticRestoration(ProstheticKind kind, const BodyPartDefinition& part) {
    CapabilityVector r{};
    const auto role = part.role;
    switch (kind) {
        case ProstheticKind::Crutch:
            r[idx(Capability::Walk)] = 0.30f;
            break;
        case ProstheticKind::Brace:
            r[idx(Capability::Walk)] = 0.20f;
            r[idx(Capability::HeavyWork)] = 0.10f;
            break;
        case ProstheticKind::MechanicalLimb:
            if (role == BodyPartRole::Leg || role == BodyPartRole::Foot) {
                r[idx(Capability::Walk)] = 0.55f;
                r[idx(Capability::HeavyWork)] = 0.20f;
                r[idx(Capability::Combat)] = 0.12f;
            } else if (role == BodyPartRole::Arm || role == BodyPartRole::Hand) {
                r[idx(Capability::Grasp)] = 0.55f;
                r[idx(Capability::FineWork)] = 0.35f;
                r[idx(Capability::HeavyWork)] = 0.20f;
                r[idx(Capability::Combat)] = 0.15f;
            }
            break;
        case ProstheticKind::CyberneticLimb:
            if (role == BodyPartRole::Leg || role == BodyPartRole::Foot) {
                r[idx(Capability::Walk)] = 0.72f;
                r[idx(Capability::HeavyWork)] = 0.28f;
                r[idx(Capability::Combat)] = 0.20f;
            } else if (role == BodyPartRole::Arm || role == BodyPartRole::Hand) {
                r[idx(Capability::Grasp)] = 0.72f;
                r[idx(Capability::FineWork)] = 0.60f;
                r[idx(Capability::HeavyWork)] = 0.30f;
                r[idx(Capability::Combat)] = 0.22f;
            }
            break;
        case ProstheticKind::SensoryAid:
            if (role == BodyPartRole::Eye) r[idx(Capability::See)] = 0.42f;
            if (role == BodyPartRole::Ear) r[idx(Capability::Hear)] = 0.42f;
            break;
        case ProstheticKind::ImplantedFilter:
            r[idx(Capability::Breathe)] = 0.25f;
            break;
        case ProstheticKind::PoweredExoskeleton:
            r[idx(Capability::Walk)] = 0.45f;
            r[idx(Capability::HeavyWork)] = 0.45f;
            r[idx(Capability::Combat)] = 0.30f;
            break;
    }
    return r;
}

bool recordContainsDiagnosis(const PatientMedicalState& p, WoundId woundId) {
    return std::any_of(p.diagnoses.begin(), p.diagnoses.end(), [woundId](const DiagnosisRecord& d){ return d.woundId == woundId; });
}

} // namespace

const BodyPlanDefinition& standardHumanoidBodyPlan() { return buildStandardHumanoid(); }

const BodyPartDefinition* findPart(const BodyPlanDefinition& plan, BodyPartId id) {
    const auto it = std::find_if(plan.parts.begin(), plan.parts.end(), [id](const BodyPartDefinition& p){ return p.id == id; });
    return it == plan.parts.end() ? nullptr : &*it;
}

std::optional<BodyPartId> findPartByName(const BodyPlanDefinition& plan, std::string_view name) {
    const auto it = std::find_if(plan.parts.begin(), plan.parts.end(), [name](const BodyPartDefinition& p){ return p.name == name; });
    if (it == plan.parts.end()) return std::nullopt;
    return it->id;
}

PatientMedicalState MedicalSystem::makePatient(StableId patientStableId, const BodyPlanDefinition& plan) {
    if (patientStableId == InvalidStableId) throw std::invalid_argument("medical patient requires nonzero stable identity");
    const auto planIssues = validateBodyPlan(plan);
    if (!planIssues.empty()) throw std::invalid_argument("invalid medical body plan: " + planIssues.front());
    PatientMedicalState p;
    p.patientStableId = patientStableId;
    p.bodyPlanContentId = plan.contentId;
    p.bodyParts.reserve(plan.parts.size());
    for (const auto& def : plan.parts) p.bodyParts.push_back({def.id});
    p.medical.recoveryProgress = 1.0f;
    rebuildCapabilities(p, plan);
    updateStatus(p);
    return p;
}

std::vector<MedicalEvent> MedicalSystem::applyInjury(PatientMedicalState& patient,
                                                     const BodyPlanDefinition& plan,
                                                     const InjurySpec& spec) {
    const auto* part = findPart(plan, spec.partId);
    auto* partState = findPartState(patient, spec.partId);
    if (!part || !partState) throw std::invalid_argument("injury references body part not present in patient plan");
    if (patient.medical.deceased) return {};

    WoundRecord wound;
    wound.id = patient.nextWoundId++;
    wound.partId = spec.partId;
    wound.kind = spec.kind;
    wound.severity = spec.severity;
    wound.tissueDamage = spec.tissueDamage > 0.0f ? clamp01(spec.tissueDamage) : severityDamage(spec.severity);
    wound.pain = severityPain(spec.severity);
    wound.contamination = clamp01(spec.contamination);
    // Negative means derive. Zero is a deliberate bloodless injury.
    wound.bleedPerMinute = spec.bleedPerMinute < 0.0f ? defaultBleed(*part, spec.kind, spec.severity)
                                                     : std::max(0.0f, spec.bleedPerMinute);
    wound.infectionRisk = clamp01(0.04f + wound.contamination * 0.55f + (isOpenWound(spec.kind) ? 0.12f : 0.0f));
    wound.surgeryRequired = needsSurgery(spec.kind, spec.severity);
    wound.boneCareRequired = spec.kind == InjuryKind::Fracture;
    wound.closureRequired = isOpenWound(spec.kind);
    wound.medicationRequired = requiresMedication(spec.kind, wound.contamination);
    wound.bleedingControlled = wound.bleedPerMinute <= 0.0001f;
    wound.decontaminated = wound.contamination <= 0.12f;
    wound.stabilized = wound.bleedingControlled && wound.decontaminated;

    if (spec.kind == InjuryKind::Amputation || spec.severity == InjurySeverity::Missing) {
        partState->missing = true;
        partState->condition = 0.0f;
        partState->nerveFunction = 0.0f;
        wound.severity = InjurySeverity::Missing;
        wound.tissueDamage = 1.0f;
        wound.surgeryRequired = true;
        wound.closureRequired = true;
    } else {
        partState->condition = clamp01(partState->condition - wound.tissueDamage);
        if (spec.severity >= InjurySeverity::FunctionLoss)
            partState->nerveFunction = std::min(partState->nerveFunction, clamp01(1.0f - wound.tissueDamage * 0.85f));
    }

    if (spec.kind == InjuryKind::Fracture) partState->fracture = std::max(partState->fracture, wound.tissueDamage);
    partState->contamination = std::max(partState->contamination, wound.contamination);

    patient.vitals.pain = clamp01(patient.vitals.pain + wound.pain * 0.45f);
    patient.vitals.shock = clamp01(patient.vitals.shock + severityDamage(wound.severity) * 0.22f + (part->vitalWeight * wound.tissueDamage * 0.20f));
    if (part->vitalWeight > 0.9f && partState->condition <= 0.02f) {
        patient.vitals.consciousness = 0.0f;
        patient.vitals.shock = 1.0f;
    }
    if (spec.kind == InjuryKind::Irradiation || spec.kind == InjuryKind::Syndrome)
        patient.vitals.systemicToxin = clamp01(patient.vitals.systemicToxin + wound.tissueDamage * 0.5f);

    patient.wounds.push_back(wound);
    patient.medical.recoveryProgress = 0.0f;
    patient.medical.diagnosisComplete = false;
    rebuildCapabilities(patient, plan);
    updateStatus(patient);

    std::vector<MedicalEvent> events;
    const bool significant = wound.severity >= InjurySeverity::FunctionLoss || wound.kind == InjuryKind::Amputation;
    events.push_back({MedicalEventKind::CitizenWounded, patient.patientStableId, wound.id, wound.partId,
                      TreatmentKind::Diagnose, wound.tissueDamage, significant});
    if (wound.kind == InjuryKind::Infection || wound.kind == InjuryKind::Syndrome) {
        events.push_back({MedicalEventKind::SyndromeContracted, patient.patientStableId, wound.id, wound.partId,
                          TreatmentKind::AdministerMedication, wound.tissueDamage, wound.severity >= InjurySeverity::FunctionLoss});
    }
    if (significant) {
        events.push_back({MedicalEventKind::MajorDisability, patient.patientStableId, wound.id, wound.partId,
                          TreatmentKind::Rehabilitation, 1.0f - patient.capabilities.get(Capability::HeavyWork), true});
    }
    return events;
}

std::vector<MedicalEvent> MedicalSystem::applyDirectDamage(PatientMedicalState& patient,
                                                           const BodyPlanDefinition& plan,
                                                           const DirectDamageSpec& damage) {
    BodyPartId partId = damage.partId;
    if (partId == InvalidBodyPart) {
        const auto torso = findPartByName(plan, "torso");
        if (!torso) throw std::logic_error("standard direct-damage bridge requires a torso-like fallback part");
        partId = *torso;
    }
    InjurySpec spec;
    spec.partId = partId;
    spec.kind = damage.kind;
    spec.severity = severityFromDirectDamage(std::max(0.0f, damage.scalarDamage));
    spec.tissueDamage = clamp01(std::max(0.0f, damage.scalarDamage) / 100.0f);
    spec.bleedPerMinute = -1.0f;
    if (damage.kind == InjuryKind::Burn || damage.kind == InjuryKind::Irradiation || damage.kind == InjuryKind::Contamination)
        spec.bleedPerMinute = 0.0f;
    spec.contamination = damage.kind == InjuryKind::Corrosion || damage.kind == InjuryKind::Contamination ?
                         clamp01(damage.scalarDamage / 100.0f) : 0.0f;
    return applyInjury(patient, plan, spec);
}

void MedicalSystem::rebuildCapabilities(PatientMedicalState& patient, const BodyPlanDefinition& plan) {
    CapabilityVector next{};
    for (std::size_t i = 0; i < CapabilityCount; ++i) {
        const auto c = static_cast<Capability>(i);
        float value = naturalCapability(patient, plan, c);
        // Devices add a bounded restoration share. This is intentionally not a
        // full return to pristine biological function unless content specifies it.
        value = clamp01(value + prostheticFloor(patient, c));
        next[i] = value;
    }

    // Systemic state constrains effective function without corrupting learned/job data.
    const float consciousness = clamp01(patient.vitals.consciousness * (1.0f - 0.55f * patient.vitals.shock));
    next[idx(Capability::Conscious)] = std::min(next[idx(Capability::Conscious)], consciousness);
    const float oxygenFactor = clamp01((patient.vitals.oxygen - 0.15f) / 0.85f);
    next[idx(Capability::HeavyWork)] *= oxygenFactor;
    next[idx(Capability::Combat)] *= oxygenFactor;
    const float painFactor = clamp01(1.0f - patient.vitals.pain * 0.50f);
    next[idx(Capability::FineWork)] *= painFactor;
    next[idx(Capability::Combat)] *= painFactor;

    patient.capabilities.values = next;
    ++patient.capabilities.revision;
}

TriageCategory MedicalSystem::evaluateTriage(const PatientMedicalState& patient) {
    if (patient.medical.deceased || patient.vitals.blood <= 0.01f) return TriageCategory::Deceased;
    if (patient.vitals.consciousness <= 0.02f && patient.vitals.blood < 0.12f) return TriageCategory::Expectant;

    TriageCategory category = TriageCategory::Routine;
    const float bleed = totalBleed(patient);
    if (bleed > 0.10f || patient.vitals.blood < 0.35f || patient.vitals.oxygen < 0.35f || patient.vitals.shock > 0.70f)
        category = TriageCategory::Immediate;
    else if (bleed > 0.025f || patient.vitals.blood < 0.60f || patient.vitals.shock > 0.45f)
        category = TriageCategory::Urgent;

    for (const auto& wound : patient.wounds) {
        if (isHealed(wound)) continue;
        TriageCategory local = TriageCategory::Routine;
        switch (wound.severity) {
            case InjurySeverity::Minor: local = TriageCategory::Routine; break;
            case InjurySeverity::Inhibited: local = TriageCategory::Delayed; break;
            case InjurySeverity::FunctionLoss: local = TriageCategory::Urgent; break;
            case InjurySeverity::StructuralLoss:
            case InjurySeverity::Missing:
            case InjurySeverity::Systemic: local = TriageCategory::Immediate; break;
        }
        if (triageRank(local) > triageRank(category)) category = local;
    }
    return category;
}

std::vector<TreatmentTask> MedicalSystem::treatmentPlan(const PatientMedicalState& patient,
                                                        const BodyPlanDefinition& plan) {
    (void)plan;
    std::vector<TreatmentTask> tasks;
    if (patient.medical.deceased) return tasks;

    if (!patient.medical.inMedicalZone) tasks.push_back(makeTask(TreatmentKind::RescueToHospital));

    std::vector<const WoundRecord*> wounds;
    wounds.reserve(patient.wounds.size());
    for (const auto& w : patient.wounds) if (!isHealed(w)) wounds.push_back(&w);
    std::sort(wounds.begin(), wounds.end(), [](const WoundRecord* a, const WoundRecord* b) {
        return std::tuple{-static_cast<int>(a->severity), a->id} < std::tuple{-static_cast<int>(b->severity), b->id};
    });

    for (const auto* w : wounds) {
        if (!w->diagnosed) tasks.push_back(makeTask(TreatmentKind::Diagnose, w->id, w->partId));
    }

    // The execution engine enforces stage ordering. We still return the entire
    // desired chain so the why-inspector can show future care requirements.
    for (const auto* w : wounds) {
        if (w->bleedPerMinute > 0.0001f && !w->bleedingControlled)
            tasks.push_back(makeTask(TreatmentKind::StopBleeding, w->id, w->partId));
        if (w->contamination > 0.12f && !w->decontaminated)
            tasks.push_back(makeTask(TreatmentKind::Decontaminate, w->id, w->partId));
    }
    if (patient.vitals.oxygen < 0.70f) tasks.push_back(makeTask(TreatmentKind::RestoreAirwayOxygen));
    if (patient.vitals.shock > 0.25f) tasks.push_back(makeTask(TreatmentKind::TreatShock));

    for (const auto* w : wounds) {
        if (w->surgeryRequired && !w->surgeryComplete)
            tasks.push_back(makeTask(TreatmentKind::Surgery, w->id, w->partId));
    }
    for (const auto* w : wounds) {
        if (w->boneCareRequired && !w->boneSet)
            tasks.push_back(makeTask(TreatmentKind::BoneSetting, w->id, w->partId));
    }
    for (const auto* w : wounds) {
        if (w->closureRequired && !w->sutured)
            tasks.push_back(makeTask(TreatmentKind::Suture, w->id, w->partId));
        if (!w->dressed && (w->closureRequired || w->kind == InjuryKind::Burn || w->kind == InjuryKind::Corrosion))
            tasks.push_back(makeTask(TreatmentKind::DressWound, w->id, w->partId));
    }
    for (const auto* w : wounds) {
        if (w->medicationRequired && !w->medicated)
            tasks.push_back(makeTask(TreatmentKind::AdministerMedication, w->id, w->partId));
    }

    // Recovery remains an explicit job until all treated wounds have reached
    // their stable healed state; recoveryProgress alone is not allowed to hide
    // a still-healing body part.
    if (!wounds.empty())
        tasks.push_back(makeTask(TreatmentKind::MonitorRecovery));

    bool missingNeedsProsthetic = false;
    // Missing functional parts advertise a prosthetic task until a device is installed.
    for (const auto& partState : patient.bodyParts) {
        if (!partState.missing) continue;
        const bool already = std::any_of(patient.prosthetics.begin(), patient.prosthetics.end(), [&](const ProstheticRecord& p) {
            return p.partId == partState.partId && p.installed;
        });
        if (!already) {
            missingNeedsProsthetic = true;
            tasks.push_back(makeTask(TreatmentKind::FitProsthetic, 0, partState.partId));
        }
    }

    const bool rehabilitationNeeded = patient.capabilities.get(Capability::Walk) < 0.92f ||
                                      patient.capabilities.get(Capability::Grasp) < 0.92f ||
                                      patient.capabilities.get(Capability::See) < 0.92f ||
                                      patient.capabilities.get(Capability::Hear) < 0.92f ||
                                      patient.capabilities.get(Capability::FineWork) < 0.92f;
    // Fit missing-part adaptations before ordinary therapy so a permanently
    // absent function cannot create an endless rehabilitation loop.
    bool prostheticServiceNeeded = false;
    for (const auto& device : patient.prosthetics) {
        if (!device.installed) continue;
        if (device.maintenance >= 0.65f || device.condition < 0.70f) {
            prostheticServiceNeeded = true;
            tasks.push_back(makeTask(TreatmentKind::ServiceProsthetic, 0, device.partId));
        }
    }

    if (rehabilitationNeeded && patient.medical.medicallyStable && !missingNeedsProsthetic && !prostheticServiceNeeded)
        tasks.push_back(makeTask(TreatmentKind::Rehabilitation));

    sortPlan(tasks);
    return tasks;
}

TreatmentResult MedicalSystem::executeTreatment(PatientMedicalState& patient,
                                                const BodyPlanDefinition& plan,
                                                const TreatmentTask& task,
                                                const TreatmentExecutionContext& context,
                                                IMedicalReservationGateway& reservations,
                                                std::vector<MedicalEvent>* emittedEvents) {
    TreatmentResult result;
    if (patient.medical.deceased) { result.reason = "patient is deceased"; return result; }
    if (context.jobStableId == InvalidStableId) { result.reason = "treatment job requires stable identity"; return result; }
    if (context.worker.stableId == InvalidStableId) { result.reason = "no qualified worker selected"; return result; }

    // A queued job is only an intent. Re-read current patient state and select
    // the canonical task definition so stale/forged requirement fields cannot
    // skip stage dependencies, skill gates, beds, facilities or consumables.
    const auto planNow = treatmentPlan(patient, plan);
    if (planNow.empty()) {
        result.reason = "patient has no currently indicated treatment";
        patient.medical.blockedReason = result.reason;
        return result;
    }
    const auto matching = std::find_if(planNow.begin(), planNow.end(), [&](const TreatmentTask& candidate) {
        return candidate.kind == task.kind && candidate.woundId == task.woundId && candidate.partId == task.partId;
    });
    if (matching == planNow.end()) {
        result.reason = "treatment is stale or no longer indicated";
        patient.medical.blockedReason = result.reason;
        return result;
    }
    const TreatmentTask canonical = *matching;
    const TreatmentStage earliestStage = planNow.front().stage;
    if (canonical.stage != earliestStage) {
        result.reason = "earlier treatment stage remains unresolved";
        patient.medical.blockedReason = result.reason;
        return result;
    }

    if (context.worker.skill(canonical.skill) + 1e-6f < canonical.minimumSkill) {
        result.reason = "selected worker lacks required medical skill";
        patient.medical.blockedReason = result.reason;
        return result;
    }
    if (canonical.needsBed && context.bedStableId == InvalidStableId) {
        result.reason = "treatment requires a patient bed";
        patient.medical.blockedReason = result.reason;
        return result;
    }
    if (canonical.needsFacility && context.facilityStableId == InvalidStableId) {
        result.reason = "treatment requires a compatible facility";
        patient.medical.blockedReason = result.reason;
        return result;
    }
    if ((canonical.kind == TreatmentKind::FitProsthetic || canonical.kind == TreatmentKind::ServiceProsthetic) &&
        context.deviceStableId == InvalidStableId) {
        result.reason = "prosthetic work requires a stable device object";
        patient.medical.blockedReason = result.reason;
        return result;
    }

    WoundRecord* wound = taskWound(patient, canonical);
    if (canonical.woundId != 0 && !wound) {
        result.reason = "treatment references unknown wound";
        patient.medical.blockedReason = result.reason;
        return result;
    }

    // Validate device work before reserving/consuming any material. Owner-thread
    // commit serialization then guarantees this stays valid for the mutation.
    ProstheticRecord* deviceToService = nullptr;
    if (canonical.kind == TreatmentKind::FitProsthetic) {
        auto* ps = findPartState(patient, canonical.partId);
        const auto* def = findPart(plan, canonical.partId);
        if (!ps || !def || !ps->missing) {
            result.reason = "prosthetic target is not a missing body part";
            patient.medical.blockedReason = result.reason;
            return result;
        }
        const bool duplicate = std::any_of(patient.prosthetics.begin(), patient.prosthetics.end(), [&](const ProstheticRecord& p) {
            return p.deviceStableId == context.deviceStableId || (p.partId == canonical.partId && p.installed);
        });
        if (duplicate) {
            result.reason = "body part already has a fitted device";
            patient.medical.blockedReason = result.reason;
            return result;
        }
    } else if (canonical.kind == TreatmentKind::ServiceProsthetic) {
        const auto it = std::find_if(patient.prosthetics.begin(), patient.prosthetics.end(), [&](const ProstheticRecord& p) {
            return p.installed && p.partId == canonical.partId && p.deviceStableId == context.deviceStableId;
        });
        if (it == patient.prosthetics.end()) {
            result.reason = "selected prosthetic device is not installed on the target part";
            patient.medical.blockedReason = result.reason;
            return result;
        }
        deviceToService = &*it;
    }

    result.reservations.push_back({ResourceKind::Worker, context.worker.stableId, 0, 1, true});
    if (canonical.needsBed) result.reservations.push_back({ResourceKind::Bed, context.bedStableId, 0, 1, true});
    if (canonical.needsFacility) result.reservations.push_back({ResourceKind::Facility, context.facilityStableId, 0, 1, true});
    if (canonical.kind == TreatmentKind::FitProsthetic || canonical.kind == TreatmentKind::ServiceProsthetic)
        result.reservations.push_back({ResourceKind::Device, context.deviceStableId, 0, 1, true});
    for (const auto& [supply, count] : canonical.supplies)
        result.reservations.push_back({ResourceKind::Supply, 0, static_cast<std::uint32_t>(supply), count, false});

    const auto attempt = reservations.tryReserveAll(context.jobStableId, result.reservations);
    if (!attempt.success) {
        result.reason = attempt.reason.empty() ? "medical resource reservation failed" : attempt.reason;
        patient.medical.blockedReason = result.reason;
        return result;
    }

    const float quality = clamp01(0.45f + context.worker.skill(canonical.skill) * 0.55f);
    result.quality = quality;
    bool consumed = true;
    if (!canonical.supplies.empty()) consumed = reservations.commitConsumables(context.jobStableId);
    if (!consumed) {
        reservations.release(context.jobStableId);
        result.reason = "reserved medical consumables could not be committed";
        patient.medical.blockedReason = result.reason;
        return result;
    }

    switch (canonical.kind) {
        case TreatmentKind::RescueToHospital:
            patient.medical.inMedicalZone = true;
            patient.medical.hospitalStableId = context.facilityStableId;
            patient.medical.bedStableId = context.bedStableId;
            break;
        case TreatmentKind::Diagnose:
            if (wound) {
                wound->diagnosed = true;
                if (!recordContainsDiagnosis(patient, wound->id))
                    patient.diagnoses.push_back({wound->id, wound->kind, wound->severity, patient.nextRecordSequence++});
                addEvent(emittedEvents, {MedicalEventKind::DiagnosisMade, patient.patientStableId, wound->id, wound->partId,
                                         canonical.kind, quality, wound->severity >= InjurySeverity::FunctionLoss});
            }
            patient.medical.diagnosisComplete = std::all_of(patient.wounds.begin(), patient.wounds.end(), [](const WoundRecord& w) {
                return isHealed(w) || w.diagnosed;
            });
            break;
        case TreatmentKind::StopBleeding:
            if (wound) {
                wound->bleedingControlled = true;
                wound->bleedPerMinute *= 0.05f;
                wound->stabilized = wound->bleedingControlled && (wound->decontaminated || wound->contamination <= 0.12f);
            }
            break;
        case TreatmentKind::RestoreAirwayOxygen:
            patient.vitals.oxygen = std::max(patient.vitals.oxygen, 0.88f + 0.10f * quality);
            break;
        case TreatmentKind::Decontaminate:
            if (wound) {
                wound->contamination *= (1.0f - 0.85f * quality);
                if (auto* ps = findPartState(patient, wound->partId)) ps->contamination = wound->contamination;
                if (wound->contamination < 0.12f) wound->decontaminated = true;
                wound->stabilized = wound->bleedingControlled && wound->decontaminated;
            }
            break;
        case TreatmentKind::TreatShock:
            patient.vitals.shock = std::max(0.0f, patient.vitals.shock - (0.42f + 0.35f * quality));
            patient.vitals.consciousness = std::max(patient.vitals.consciousness, 0.50f + 0.35f * quality);
            break;
        case TreatmentKind::Surgery:
            if (wound) {
                wound->surgeryComplete = true;
                wound->stabilized = wound->bleedingControlled && wound->decontaminated;
                if (auto* ps = findPartState(patient, wound->partId)) {
                    ps->surgicallyStabilized = true;
                    if (!ps->missing) {
                        ps->condition = clamp01(ps->condition + (0.12f + 0.18f * quality));
                        ps->nerveFunction = clamp01(ps->nerveFunction + 0.08f * quality);
                    }
                }
            }
            break;
        case TreatmentKind::BoneSetting:
            if (wound) {
                wound->boneSet = true;
                if (auto* ps = findPartState(patient, wound->partId)) {
                    ps->boneSet = true;
                    ps->fracture *= 0.15f;
                    if (!ps->missing) ps->condition = clamp01(ps->condition + 0.08f * quality);
                }
            }
            break;
        case TreatmentKind::Suture:
            if (wound) {
                wound->sutured = true;
                wound->bleedingControlled = true;
                wound->bleedPerMinute *= 0.05f;
                wound->stabilized = wound->bleedingControlled && (wound->decontaminated || wound->contamination <= 0.12f);
                wound->infectionRisk *= 0.75f;
            }
            break;
        case TreatmentKind::DressWound:
            if (wound) {
                wound->dressed = true;
                wound->infectionRisk *= 0.60f;
                wound->pain *= 0.88f;
            }
            break;
        case TreatmentKind::AdministerMedication:
            if (wound) {
                wound->medicated = true;
                wound->infectionRisk *= 0.28f;
                if (wound->kind == InjuryKind::Infection || wound->kind == InjuryKind::Syndrome)
                    wound->healing = std::max(wound->healing, 0.20f * quality);
            }
            patient.vitals.systemicToxin *= (1.0f - 0.45f * quality);
            break;
        case TreatmentKind::MonitorRecovery:
            patient.medical.recoveryProgress = clamp01(patient.medical.recoveryProgress + 0.28f + 0.30f * quality);
            patient.vitals.blood = clamp01(patient.vitals.blood + 0.12f + 0.10f * quality);
            patient.vitals.pain = clamp01(patient.vitals.pain - 0.12f - 0.10f * quality);
            patient.vitals.shock = clamp01(patient.vitals.shock - 0.10f - 0.08f * quality);
            for (auto& w : patient.wounds) {
                if (allRequiredCareComplete(patient) || w.stabilized) {
                    const float before = w.healing;
                    w.healing = clamp01(w.healing + 0.18f + 0.18f * quality);
                    const float healedShare = w.healing - before;
                    if (auto* ps = findPartState(patient, w.partId); ps && !ps->missing) {
                        ps->condition = clamp01(ps->condition + healedShare * w.tissueDamage * 0.85f);
                        ps->nerveFunction = clamp01(ps->nerveFunction + healedShare * w.tissueDamage * 0.45f);
                    }
                }
            }
            break;
        case TreatmentKind::Rehabilitation:
            patient.medical.recoveryProgress = clamp01(patient.medical.recoveryProgress + 0.15f + 0.20f * quality);
            for (auto& ps : patient.bodyParts) if (!ps.missing)
                ps.nerveFunction = clamp01(ps.nerveFunction + 0.04f * quality);
            break;
        case TreatmentKind::FitProsthetic: {
            const auto* def = findPart(plan, canonical.partId);
            ProstheticRecord device;
            device.deviceStableId = context.deviceStableId;
            device.partId = canonical.partId;
            device.kind = context.prostheticKind;
            device.restoration = defaultProstheticRestoration(context.prostheticKind, *def);
            device.requiresPower = context.prostheticKind == ProstheticKind::CyberneticLimb ||
                                   context.prostheticKind == ProstheticKind::PoweredExoskeleton;
            device.powered = true;
            device.installed = true;
            patient.prosthetics.push_back(device);
            addEvent(emittedEvents, {MedicalEventKind::ProstheticInstalled, patient.patientStableId, 0, canonical.partId,
                                     canonical.kind, quality, true});
            break;
        }
        case TreatmentKind::ServiceProsthetic:
            deviceToService->maintenance = std::max(0.0f, deviceToService->maintenance - (0.70f + 0.25f * quality));
            deviceToService->condition = clamp01(deviceToService->condition + 0.18f + 0.20f * quality);
            addEvent(emittedEvents, {MedicalEventKind::ProstheticServiced, patient.patientStableId, 0, canonical.partId,
                                     canonical.kind, quality, false});
            break;
    }

    patient.treatments.push_back({canonical.kind, canonical.woundId, context.worker.stableId, patient.nextRecordSequence++, quality});
    addEvent(emittedEvents, {MedicalEventKind::TreatmentPerformed, patient.patientStableId, canonical.woundId, canonical.partId,
                             canonical.kind, quality,
                             canonical.kind == TreatmentKind::Surgery || canonical.kind == TreatmentKind::FitProsthetic});

    if (wound && !wound->scarRecorded && wound->severity >= InjurySeverity::FunctionLoss &&
        (wound->surgeryComplete || wound->sutured || wound->boneSet)) {
        patient.scars.push_back({wound->id, wound->partId, wound->kind, severityDamage(wound->severity)});
        wound->scarRecorded = true;
        addEvent(emittedEvents, {MedicalEventKind::ScarFormed, patient.patientStableId, wound->id, wound->partId,
                                 canonical.kind, severityDamage(wound->severity), true});
    }

    reservations.release(context.jobStableId);
    patient.medical.blockedReason.clear();
    rebuildCapabilities(patient, plan);
    updateStatus(patient);
    result.success = true;
    return result;
}

std::vector<MedicalEvent> MedicalSystem::advancePhysiology(PatientMedicalState& patient,
                                                           const BodyPlanDefinition& plan,
                                                           float elapsedSeconds) {
    if (elapsedSeconds < 0.0f || !std::isfinite(elapsedSeconds)) throw std::invalid_argument("medical physiology requires finite nonnegative dt");
    std::vector<MedicalEvent> events;
    if (patient.medical.deceased || elapsedSeconds == 0.0f) return events;
    const float minutes = elapsedSeconds / 60.0f;

    patient.vitals.blood = clamp01(patient.vitals.blood - totalBleed(patient) * minutes);
    const float averagePain = [&] {
        float p = 0.0f;
        for (const auto& w : patient.wounds) if (!isHealed(w)) p += w.pain * (1.0f - w.healing);
        return clamp01(p * 0.30f);
    }();
    patient.vitals.pain = clamp01(std::max(patient.vitals.pain * std::exp(-0.012f * minutes), averagePain));

    if (patient.vitals.blood < 0.55f) patient.vitals.shock = clamp01(patient.vitals.shock + (0.55f - patient.vitals.blood) * 0.06f * minutes);
    else if (patient.medical.stabilizationComplete) patient.vitals.shock = clamp01(patient.vitals.shock - 0.018f * minutes);
    patient.vitals.consciousness = clamp01(1.0f - patient.vitals.shock * 0.75f - std::max(0.0f, 0.35f - patient.vitals.blood) * 1.8f);

    for (auto& w : patient.wounds) {
        if (isHealed(w)) continue;
        if (!w.dressed && w.closureRequired) w.infectionRisk = clamp01(w.infectionRisk + 0.0025f * minutes * (1.0f + w.contamination));
        if (w.medicated) w.infectionRisk = clamp01(w.infectionRisk - 0.0040f * minutes);
        if (w.infectionRisk >= 0.88f && w.kind != InjuryKind::Infection) {
            // Deterministic threshold conversion avoids hidden RNG in save-relevant physiology.
            w.kind = InjuryKind::Infection;
            w.medicationRequired = true;
            w.medicated = false;
            events.push_back({MedicalEventKind::SyndromeContracted, patient.patientStableId, w.id, w.partId,
                              TreatmentKind::AdministerMedication, w.infectionRisk, w.severity >= InjurySeverity::FunctionLoss});
        }
        const bool careReady = w.diagnosed && (!w.surgeryRequired || w.surgeryComplete) &&
                               (!w.boneCareRequired || w.boneSet) &&
                               (!w.closureRequired || w.sutured || w.dressed) &&
                               (!w.medicationRequired || w.medicated);
        if (careReady && patient.medical.medicallyStable) {
            w.healing = clamp01(w.healing + 0.006f * minutes);
            if (auto* ps = findPartState(patient, w.partId); ps && !ps->missing)
                ps->condition = clamp01(ps->condition + 0.0025f * minutes);
        }
    }

    const float hours = elapsedSeconds / 3600.0f;
    for (auto& device : patient.prosthetics) {
        if (!device.installed) continue;
        float rate = 0.004f;
        switch (device.kind) {
            case ProstheticKind::Crutch: rate = 0.002f; break;
            case ProstheticKind::Brace: rate = 0.0025f; break;
            case ProstheticKind::MechanicalLimb: rate = 0.008f; break;
            case ProstheticKind::CyberneticLimb: rate = 0.012f; break;
            case ProstheticKind::SensoryAid: rate = 0.006f; break;
            case ProstheticKind::ImplantedFilter: rate = 0.005f; break;
            case ProstheticKind::PoweredExoskeleton: rate = 0.015f; break;
        }
        device.maintenance = clamp01(device.maintenance + rate * hours);
        if (device.maintenance > 0.70f)
            device.condition = clamp01(device.condition - (device.maintenance - 0.70f) * 0.012f * hours);
    }

    if (patient.vitals.blood <= 0.01f) {
        patient.medical.deceased = true;
        patient.vitals.consciousness = 0.0f;
        events.push_back({MedicalEventKind::CitizenDied, patient.patientStableId, 0, InvalidBodyPart,
                          TreatmentKind::MonitorRecovery, 1.0f, true});
    }

    rebuildCapabilities(patient, plan);
    updateStatus(patient);
    return events;
}

float MedicalSystem::scalarHealthEquivalent(const PatientMedicalState& patient,
                                            const BodyPlanDefinition& plan,
                                            float maxHealth) {
    float vitalNumerator = 0.0f;
    float vitalDenominator = 0.0f;
    for (const auto& def : plan.parts) {
        if (def.vitalWeight <= 0.0f) continue;
        vitalDenominator += def.vitalWeight;
        const auto* state = findPartState(patient, def.id);
        const float condition = state && !state->missing ? clamp01(state->condition) : 0.0f;
        vitalNumerator += def.vitalWeight * condition;
    }
    const float vital = vitalDenominator > 0.0f ? vitalNumerator / vitalDenominator : 1.0f;
    const float systemic = clamp01(0.40f * vital + 0.25f * patient.vitals.blood + 0.15f * patient.vitals.oxygen +
                                   0.10f * patient.vitals.consciousness + 0.10f * (1.0f - patient.vitals.shock));
    return std::max(0.0f, maxHealth) * systemic;
}


std::vector<std::string> MedicalSystem::validateBodyPlan(const BodyPlanDefinition& plan) {
    std::vector<std::string> issues;
    if (plan.contentId.empty()) issues.push_back("body plan content ID is empty");
    if (plan.version == 0) issues.push_back("body plan version must be nonzero");
    if (plan.parts.empty()) issues.push_back("body plan has no parts");

    std::unordered_set<BodyPartId> ids;
    std::unordered_set<std::string> names;
    for (const auto& part : plan.parts) {
        if (part.id == InvalidBodyPart) issues.push_back("body plan contains InvalidBodyPart");
        if (!ids.insert(part.id).second) issues.push_back("duplicate body-part ID " + std::to_string(part.id));
        if (part.name.empty()) issues.push_back("body part " + std::to_string(part.id) + " has empty name");
        else if (!names.insert(part.name).second) issues.push_back("duplicate body-part name " + part.name);
        if (!std::isfinite(part.vitalWeight) || part.vitalWeight < 0.0f || part.vitalWeight > 1.0f)
            issues.push_back("body part " + std::to_string(part.id) + " has invalid vital weight");
        if (!std::isfinite(part.bleedingScale) || part.bleedingScale < 0.0f)
            issues.push_back("body part " + std::to_string(part.id) + " has invalid bleeding scale");
        for (float value : part.contribution) {
            if (!std::isfinite(value) || value < 0.0f || value > 1.0f) {
                issues.push_back("body part " + std::to_string(part.id) + " has invalid capability contribution");
                break;
            }
        }
    }
    for (const auto& part : plan.parts) {
        if (part.parent != InvalidBodyPart && ids.count(part.parent) == 0)
            issues.push_back("body part " + std::to_string(part.id) + " has unknown parent");
        if (part.parent == part.id) issues.push_back("body part " + std::to_string(part.id) + " is its own parent");
        BodyPartId cursor = part.parent;
        std::size_t depth = 0;
        while (cursor != InvalidBodyPart && depth <= plan.parts.size()) {
            if (cursor == part.id) {
                issues.push_back("body-plan parent cycle includes part " + std::to_string(part.id));
                break;
            }
            const auto* parent = findPart(plan, cursor);
            if (!parent) break;
            cursor = parent->parent;
            ++depth;
        }
    }
    return issues;
}

std::vector<std::string> MedicalSystem::validatePersistentState(const MedicalPersistentState& state,
                                                                 const BodyPlanDefinition& plan) {
    std::vector<std::string> issues = validateBodyPlan(plan);
    if (state.schemaVersion != MedicalSnapshotSchemaVersion) issues.push_back("unsupported medical snapshot schema");
    if (state.patientStableId == InvalidStableId) issues.push_back("medical snapshot has invalid stable patient identity");
    if (state.bodyPlanContentId != plan.contentId) issues.push_back("medical snapshot body-plan content ID mismatch");

    auto finite01 = [&](float value, std::string_view label) {
        if (!std::isfinite(value) || value < 0.0f || value > 1.0f) issues.emplace_back(label);
    };
    finite01(state.vitals.blood, "invalid blood value");
    finite01(state.vitals.oxygen, "invalid oxygen value");
    finite01(state.vitals.pain, "invalid pain value");
    finite01(state.vitals.consciousness, "invalid consciousness value");
    finite01(state.vitals.shock, "invalid shock value");
    finite01(state.vitals.temperature, "invalid temperature value");
    finite01(state.vitals.systemicToxin, "invalid systemic toxin value");

    std::unordered_set<BodyPartId> bodyIds;
    for (const auto& part : state.bodyParts) {
        if (!findPart(plan, part.partId)) issues.push_back("snapshot contains unknown body-part ID " + std::to_string(part.partId));
        if (!bodyIds.insert(part.partId).second) issues.push_back("snapshot duplicates body-part ID " + std::to_string(part.partId));
        finite01(part.condition, "invalid body-part condition");
        finite01(part.nerveFunction, "invalid body-part nerve function");
        finite01(part.contamination, "invalid body-part contamination");
        finite01(part.fracture, "invalid body-part fracture value");
    }
    for (const auto& def : plan.parts) if (bodyIds.count(def.id) == 0)
        issues.push_back("snapshot is missing body-part ID " + std::to_string(def.id));

    std::unordered_set<WoundId> woundIds;
    WoundId maxWound = 0;
    for (const auto& wound : state.wounds) {
        if (wound.id == 0 || !woundIds.insert(wound.id).second) issues.push_back("snapshot has invalid or duplicate wound ID");
        maxWound = std::max(maxWound, wound.id);
        if (!findPart(plan, wound.partId)) issues.push_back("wound references unknown body part");
        finite01(wound.tissueDamage, "invalid wound tissue damage");
        if (!std::isfinite(wound.bleedPerMinute) || wound.bleedPerMinute < 0.0f) issues.push_back("invalid wound bleeding rate");
        finite01(wound.pain, "invalid wound pain");
        finite01(wound.contamination, "invalid wound contamination");
        finite01(wound.infectionRisk, "invalid wound infection risk");
        finite01(wound.healing, "invalid wound healing");
    }
    if (state.nextWoundId == 0 || state.nextWoundId <= maxWound) issues.push_back("next wound ID would collide with persisted wound identity");

    std::unordered_set<StableId> deviceIds;
    for (const auto& device : state.prosthetics) {
        if (device.deviceStableId == InvalidStableId || !deviceIds.insert(device.deviceStableId).second)
            issues.push_back("snapshot has invalid or duplicate prosthetic stable ID");
        if (!findPart(plan, device.partId)) issues.push_back("prosthetic references unknown body part");
        finite01(device.condition, "invalid prosthetic condition");
        finite01(device.maintenance, "invalid prosthetic maintenance");
        for (float value : device.restoration) if (!std::isfinite(value) || value < 0.0f || value > 1.0f) {
            issues.push_back("invalid prosthetic capability restoration");
            break;
        }
    }
    for (const auto& diagnosis : state.diagnoses) if (woundIds.count(diagnosis.woundId) == 0)
        issues.push_back("diagnosis references unknown wound");
    std::uint32_t maxSequence = 0;
    for (const auto& diagnosis : state.diagnoses) maxSequence = std::max(maxSequence, diagnosis.sequence);
    for (const auto& treatment : state.treatments) {
        if (treatment.woundId != 0 && woundIds.count(treatment.woundId) == 0) issues.push_back("treatment references unknown wound");
        if (!std::isfinite(treatment.quality) || treatment.quality < 0.0f || treatment.quality > 1.0f) issues.push_back("invalid treatment quality");
        maxSequence = std::max(maxSequence, treatment.sequence);
    }
    if (state.nextRecordSequence == 0 || state.nextRecordSequence <= maxSequence)
        issues.push_back("next medical record sequence would collide with persisted record");
    finite01(state.medical.recoveryProgress, "invalid medical recovery progress");
    return issues;
}

MedicalInspectionReport MedicalSystem::inspect(const PatientMedicalState& patient, const BodyPlanDefinition& plan) {
    MedicalInspectionReport report;
    report.triage = evaluateTriage(patient);
    report.readyForWork = patient.medical.readyForWork;
    report.medicallyStable = patient.medical.medicallyStable;
    report.capabilities = patient.capabilities.values;
    for (const auto& wound : patient.wounds) {
        if (isHealed(wound)) continue;
        ++report.activeWoundCount;
        report.uncontrolledBleeding |= wound.bleedPerMinute > 0.0001f && !wound.bleedingControlled;
        report.unmanagedContamination |= wound.contamination > 0.12f && !wound.decontaminated;
        report.infectionOrSyndrome |= wound.kind == InjuryKind::Infection || wound.kind == InjuryKind::Syndrome;
    }
    for (const auto& device : patient.prosthetics) {
        if (device.installed && (device.maintenance >= 0.65f || device.condition < 0.70f)) report.prostheticServiceDue = true;
    }
    const auto planNow = treatmentPlan(patient, plan);
    if (!planNow.empty()) report.nextTreatment = planNow.front();
    if (patient.medical.deceased) report.reasons.push_back("patient is deceased");
    if (report.uncontrolledBleeding) report.reasons.push_back("uncontrolled bleeding");
    if (report.unmanagedContamination) report.reasons.push_back("unmanaged contamination");
    if (patient.vitals.oxygen < 0.70f) report.reasons.push_back("oxygen below safe treatment threshold");
    if (patient.vitals.shock > 0.25f) report.reasons.push_back("shock requires stabilization");
    if (report.infectionOrSyndrome) report.reasons.push_back("active infection or syndrome");
    if (report.prostheticServiceDue) report.reasons.push_back("prosthetic service due");
    if (!patient.medical.blockedReason.empty()) report.reasons.push_back(patient.medical.blockedReason);
    if (!report.readyForWork && report.reasons.empty()) report.reasons.push_back("capability or recovery threshold prevents work readiness");
    return report;
}

bool MedicalSystem::setProstheticPowered(PatientMedicalState& patient,
                                          const BodyPlanDefinition& plan,
                                          StableId deviceStableId,
                                          bool powered,
                                          std::vector<MedicalEvent>* emittedEvents) {
    const auto it = std::find_if(patient.prosthetics.begin(), patient.prosthetics.end(), [&](const ProstheticRecord& p) {
        return p.deviceStableId == deviceStableId;
    });
    if (it == patient.prosthetics.end() || !it->installed || !it->requiresPower) return false;
    if (it->powered == powered) return true;
    const auto before = patient.capabilities.values;
    it->powered = powered;
    rebuildCapabilities(patient, plan);
    updateStatus(patient);
    float magnitude = 0.0f;
    for (std::size_t i = 0; i < CapabilityCount; ++i) magnitude = std::max(magnitude, std::fabs(patient.capabilities.values[i] - before[i]));
    addEvent(emittedEvents, {MedicalEventKind::CapabilityChanged, patient.patientStableId, 0, it->partId,
                             TreatmentKind::ServiceProsthetic, magnitude, false});
    return true;
}

MedicalPersistentState MedicalSystem::capturePersistentState(const PatientMedicalState& patient) {
    MedicalPersistentState s;
    s.patientStableId = patient.patientStableId;
    s.bodyPlanContentId = patient.bodyPlanContentId;
    s.vitals = patient.vitals;
    s.bodyParts = patient.bodyParts;
    s.wounds = patient.wounds;
    s.scars = patient.scars;
    s.prosthetics = patient.prosthetics;
    s.diagnoses = patient.diagnoses;
    s.treatments = patient.treatments;
    s.medical = patient.medical;
    s.nextWoundId = patient.nextWoundId;
    s.nextRecordSequence = patient.nextRecordSequence;
    return s;
}

PatientMedicalState MedicalSystem::restorePersistentState(const MedicalPersistentState& state,
                                                           const BodyPlanDefinition& plan) {
    const auto issues = validatePersistentState(state, plan);
    if (!issues.empty()) throw std::runtime_error("invalid medical snapshot: " + issues.front());

    PatientMedicalState p;
    p.patientStableId = state.patientStableId;
    p.bodyPlanContentId = state.bodyPlanContentId;
    p.vitals = state.vitals;
    p.bodyParts = state.bodyParts;
    p.wounds = state.wounds;
    p.scars = state.scars;
    p.prosthetics = state.prosthetics;
    p.diagnoses = state.diagnoses;
    p.treatments = state.treatments;
    p.medical = state.medical;
    p.nextWoundId = state.nextWoundId;
    p.nextRecordSequence = state.nextRecordSequence;

    // Derived capability cache is deliberately not save truth.
    rebuildCapabilities(p, plan);
    updateStatus(p);
    return p;
}

const WoundRecord* MedicalSystem::findWound(const PatientMedicalState& patient, WoundId id) {
    const auto it = std::find_if(patient.wounds.begin(), patient.wounds.end(), [id](const WoundRecord& w){ return w.id == id; });
    return it == patient.wounds.end() ? nullptr : &*it;
}

WoundRecord* MedicalSystem::findWound(PatientMedicalState& patient, WoundId id) {
    const auto it = std::find_if(patient.wounds.begin(), patient.wounds.end(), [id](const WoundRecord& w){ return w.id == id; });
    return it == patient.wounds.end() ? nullptr : &*it;
}

const BodyPartState* MedicalSystem::findPartState(const PatientMedicalState& patient, BodyPartId id) {
    const auto it = std::find_if(patient.bodyParts.begin(), patient.bodyParts.end(), [id](const BodyPartState& p){ return p.partId == id; });
    return it == patient.bodyParts.end() ? nullptr : &*it;
}

BodyPartState* MedicalSystem::findPartState(PatientMedicalState& patient, BodyPartId id) {
    const auto it = std::find_if(patient.bodyParts.begin(), patient.bodyParts.end(), [id](const BodyPartState& p){ return p.partId == id; });
    return it == patient.bodyParts.end() ? nullptr : &*it;
}

const char* treatmentKindName(TreatmentKind kind) {
    switch (kind) {
        case TreatmentKind::RescueToHospital: return "rescue";
        case TreatmentKind::Diagnose: return "diagnosis";
        case TreatmentKind::StopBleeding: return "stop bleeding";
        case TreatmentKind::RestoreAirwayOxygen: return "airway/oxygen";
        case TreatmentKind::Decontaminate: return "decontamination";
        case TreatmentKind::TreatShock: return "shock stabilization";
        case TreatmentKind::Surgery: return "surgery";
        case TreatmentKind::BoneSetting: return "bone setting";
        case TreatmentKind::Suture: return "suturing";
        case TreatmentKind::DressWound: return "wound dressing";
        case TreatmentKind::AdministerMedication: return "medication";
        case TreatmentKind::MonitorRecovery: return "recovery monitoring";
        case TreatmentKind::Rehabilitation: return "rehabilitation";
        case TreatmentKind::FitProsthetic: return "prosthetic fitting";
        case TreatmentKind::ServiceProsthetic: return "prosthetic service";
    }
    return "treatment";
}

const char* triageName(TriageCategory category) {
    switch (category) {
        case TriageCategory::Routine: return "routine";
        case TriageCategory::Delayed: return "delayed";
        case TriageCategory::Urgent: return "urgent";
        case TriageCategory::Immediate: return "immediate";
        case TriageCategory::Expectant: return "expectant";
        case TriageCategory::Deceased: return "deceased";
    }
    return "unknown";
}

} // namespace elysium::medical
