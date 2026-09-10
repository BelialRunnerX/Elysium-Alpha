#include "fortress/Systems.hpp"

#include <algorithm>

namespace elysium::fortress {
namespace {
float skillBySuffix(const Skills& skills, std::string_view suffix) {
    float best{};
    for (const auto& skill : skills.entries) {
        if (skill.skill.value.ends_with(suffix)) best = std::max(best, skill.effectiveRank);
    }
    return best;
}
}

float officeSuitability(const OfficeState& office, const Skills& skills, const Values& values,
                        const Personality& personality) {
    float administrative = std::max({skillBySuffix(skills, "leadership"), skillBySuffix(skills, "bookkeeping"),
                                     skillBySuffix(skills, "law"), skillBySuffix(skills, "brokerage")});
    float valueMatch = values.law * 0.25f + values.tradition * 0.15f + values.knowledge * 0.15f +
                       values.independence * 0.10f + values.family * 0.05f;
    float temperament = personality.discipline * 0.30f + personality.patience * 0.22f +
                        personality.sociability * 0.18f + (1.0f - personality.anger) * 0.12f;
    float requirementPenalty = office.requirements.empty() ? 0.0f : std::min(0.4f, static_cast<float>(office.requirements.size()) * 0.03f);
    return std::clamp(administrative * 0.035f + valueMatch + temperament - requirementPenalty, 0.0f, 2.0f);
}

float institutionServiceQuality(const InstitutionState& institution, float staffing,
                                float supplyFraction, float roomQuality) {
    const float scheduleFactor = institution.schedule.empty() ? 0.8f : 1.0f;
    return std::clamp(saturate(staffing) * 0.28f + saturate(supplyFraction) * 0.25f + saturate(roomQuality) * 0.22f +
                      saturate(institution.quality) * 0.15f + saturate(institution.prestige) * 0.10f, 0.0f, 1.5f) * scheduleFactor;
}

MigrationDecision decideMigration(const MigrationCandidate& candidate, float destinationStability,
                                  float destinationCapacity) {
    const float score = candidate.attraction * 0.28f + candidate.kinship * 0.20f + candidate.opportunity * 0.24f +
                        candidate.ideologicalFit * 0.14f + saturate(destinationStability) * 0.18f +
                        saturate(destinationCapacity) * 0.16f - candidate.fear * 0.34f;
    MigrationDecision result{};
    result.score = score;
    result.migrate = score >= (candidate.visitor ? 0.35f : 0.55f);
    result.reason = result.migrate ? "destination utility exceeds departure resistance" : "risk/capacity/fit insufficient";
    return result;
}

} // namespace elysium::fortress
