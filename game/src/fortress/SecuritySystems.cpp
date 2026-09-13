#include "fortress/Systems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

float squadReadiness(const SquadState& squad, float equipmentCompleteness, float healthFraction,
                     float scheduleCompliance) {
    const float staffing = squad.members.empty() ? 0.0f : saturate(static_cast<float>(squad.members.size()) / 8.0f);
    return saturate(staffing * 0.15f + saturate(equipmentCompleteness) * 0.25f + saturate(healthFraction) * 0.20f +
                    saturate(scheduleCompliance) * 0.10f + saturate(squad.training) * 0.15f + saturate(squad.ammoFraction) * 0.15f);
}

AlertLevel chooseSquadAlert(float threatStrength, float fortressDefense, float civilianRisk) {
    const float pressure = std::max(0.0f, threatStrength) / std::max(0.25f, fortressDefense + 0.25f) + saturate(civilianRisk) * 0.5f;
    if (pressure < 0.35f) return AlertLevel::Green;
    if (pressure < 0.8f) return AlertLevel::Yellow;
    if (pressure < 1.6f) return AlertLevel::Red;
    return AlertLevel::Black;
}

float evidenceStrength(const CrimeRecord& crime, float forensicQuality, float witnessReliability) {
    const float witness = std::min(1.0f, static_cast<float>(crime.witnesses.size()) * 0.18f) * saturate(witnessReliability);
    const float evidence = std::min(1.0f, static_cast<float>(crime.evidence.size()) * 0.24f) * saturate(forensicQuality);
    const float seriousness = saturate(crime.severity);
    return saturate(witness * 0.4f + evidence * 0.5f + seriousness * 0.1f);
}

std::string recommendSentence(const JusticeCase& justiceCase, float lawSeverity, float rehabilitationBias) {
    const float score = justiceCase.evidenceStrength * justiceCase.crime.severity * (0.5f + saturate(lawSeverity) * 0.8f);
    const float rehab = saturate(rehabilitationBias);
    if (score < 0.18f) return "dismiss_or_warning";
    if (score < 0.38f) return rehab > 0.55f ? "restitution_and_counseling" : "fine_or_restriction";
    if (score < 0.62f) return rehab > 0.55f ? "supervised_service" : "detention";
    if (score < 0.82f) return rehab > 0.7f ? "secure_rehabilitation" : "long_detention";
    return "exile_or_maximum_sentence";
}

} // namespace elysium::fortress
