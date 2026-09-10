#include "fortress/Systems.hpp"

#include <algorithm>

namespace elysium::fortress {

void recomputeBodyCapabilities(BodyState& body) {
    float mobility{1.0f};
    float manipulation{1.0f};
    float consciousness{1.0f};
    for (const auto& part : body.parts) {
        const float usable = part.missing ? 0.0f : saturate(part.integrity * part.function);
        if (part.part.value.find("leg") != std::string::npos || part.part.value.find("foot") != std::string::npos) {
            mobility = std::min(mobility, 0.35f + usable * 0.65f);
        }
        if (part.part.value.find("arm") != std::string::npos || part.part.value.find("hand") != std::string::npos) {
            manipulation = std::min(manipulation, 0.25f + usable * 0.75f);
        }
        if (part.part.value.find("brain") != std::string::npos || part.part.value.find("head") != std::string::npos) {
            consciousness = std::min(consciousness, 0.2f + usable * 0.8f);
        }
    }
    const float woundPain = [&] {
        float pain{};
        for (const auto& wound : body.wounds) pain += wound.pain;
        return saturate(pain * 0.12f);
    }();
    body.mobility = saturate(mobility * (1.0f - woundPain * 0.45f) * body.bloodFraction);
    body.manipulation = saturate(manipulation * (1.0f - woundPain * 0.25f));
    body.consciousness = saturate(consciousness * body.oxygenation * (0.5f + body.bloodFraction * 0.5f));
}

TreatmentPlan buildTreatmentPlan(StableId patient, const BodyState& body) {
    TreatmentPlan plan{};
    plan.patient = patient;
    float highest{};
    for (const auto& wound : body.wounds) {
        if (wound.healing >= 0.999f) continue;
        const float severity = static_cast<float>(static_cast<unsigned>(wound.severity)) / 4.0f;
        highest = std::max(highest, severity + wound.bleeding + wound.infectionRisk * 0.5f);

        auto add = [&](TreatmentKind kind, PriorityBand priority, std::string supply, std::string skill, float work) {
            TreatmentOrder order{};
            order.patient = patient;
            order.wound = wound.id;
            order.treatment = kind;
            order.priority = priority;
            order.supply = ContentId{std::move(supply)};
            order.skill = ContentId{std::move(skill)};
            order.work = work;
            plan.orders.push_back(std::move(order));
        };

        add(TreatmentKind::Diagnose, PriorityBand::High, "elysium:item/diagnostic_kit", "elysium:labor/diagnosis", 0.25f);
        if (wound.contamination > 0.15f) add(TreatmentKind::Clean, PriorityBand::Urgent, "elysium:item/antiseptic", "elysium:labor/trauma_care", 0.35f);
        if (wound.bleeding > 0.12f) add(TreatmentKind::Dress, PriorityBand::Emergency, "elysium:item/dressing", "elysium:labor/trauma_care", 0.45f);
        if (wound.severity >= InjurySeverity::Severe) add(TreatmentKind::Surgery, PriorityBand::Urgent, "elysium:item/surgical_pack", "elysium:labor/surgery", 1.2f);
        if (wound.infectionRisk > 0.35f) add(TreatmentKind::Medication, PriorityBand::High, "elysium:item/antimicrobial", "elysium:labor/pharmacology", 0.25f);
        if (wound.severity == InjurySeverity::Lost) add(TreatmentKind::ProstheticFit, PriorityBand::Normal, "elysium:item/prosthetic_kit", "elysium:labor/prosthetics", 1.5f);
    }
    plan.triage = highest > 1.25f ? PriorityBand::Emergency : highest > 0.75f ? PriorityBand::Urgent : highest > 0.35f ? PriorityBand::High : PriorityBand::Normal;
    return plan;
}

float treatmentEffectiveness(TreatmentKind treatment, float skill, float supplies, float cleanliness) {
    float base{0.6f};
    switch (treatment) {
        case TreatmentKind::Diagnose: base = 0.7f; break;
        case TreatmentKind::Clean: base = 0.8f; break;
        case TreatmentKind::Dress: base = 0.85f; break;
        case TreatmentKind::Suture: base = 0.8f; break;
        case TreatmentKind::SetBone: base = 0.65f; break;
        case TreatmentKind::Surgery: base = 0.5f; break;
        case TreatmentKind::Transfusion: base = 0.7f; break;
        case TreatmentKind::Medication: base = 0.75f; break;
        case TreatmentKind::Isolation: base = 0.9f; break;
        case TreatmentKind::Rehabilitation: base = 0.55f; break;
        case TreatmentKind::ProstheticFit: base = 0.5f; break;
    }
    const float skillFactor = 0.55f + std::min(20.0f, std::max(0.0f, skill)) * 0.035f;
    return std::clamp(base * skillFactor * (0.3f + saturate(supplies) * 0.7f) * (0.45f + saturate(cleanliness) * 0.55f), 0.0f, 1.5f);
}

void advanceHealing(BodyState& body, float days, float nutrition, float immunity, float careQuality) {
    const float d = std::max(0.0f, days);
    for (auto& wound : body.wounds) {
        const float infectionPenalty = saturate(wound.infectionRisk * (1.0f - immunity));
        const float heal = d * 0.04f * saturate(nutrition) * saturate(immunity) * (0.4f + saturate(careQuality) * 0.6f) * (1.0f - infectionPenalty);
        wound.healing = saturate(wound.healing + heal);
        wound.bleeding = std::max(0.0f, wound.bleeding - heal * 1.4f);
        wound.pain = std::max(0.0f, wound.pain - heal * 0.7f);
        wound.contamination = std::max(0.0f, wound.contamination - heal * 0.35f);
        wound.infectionRisk = saturate(wound.infectionRisk - heal * immunity * 0.25f + wound.contamination * d * 0.005f);
    }
    recomputeBodyCapabilities(body);
}

} // namespace elysium::fortress
