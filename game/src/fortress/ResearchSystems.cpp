#include "fortress/Systems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

void advanceResearch(ResearchProject& project, float researcherSkill, float focus,
                     float facilityQuality, float sampleQuality, float hours) {
    if (project.completed) return;
    const float skill = 0.3f + std::log1p(std::max(0.0f, researcherSkill)) * 0.25f;
    const float support = 0.25f + saturate(focus) * 0.25f + saturate(facilityQuality) * 0.25f + saturate(sampleQuality) * 0.25f;
    project.progress += std::max(0.0f, hours) * skill * support / std::max(0.1f, project.difficulty * 10.0f);
    if (project.progress >= 1.0f) {
        project.progress = 1.0f;
        project.completed = true;
    }
}

float knowledgeTransfer(float teacherSkill, float studentAptitude, float relationshipAffinity,
                        float institutionQuality, float hours) {
    const float teacher = std::log1p(std::max(0.0f, teacherSkill));
    const float social = std::clamp(0.7f + relationshipAffinity * 0.25f, 0.25f, 1.25f);
    return std::max(0.0f, hours) * teacher * std::max(0.1f, studentAptitude) * social *
           (0.5f + saturate(institutionQuality) * 0.5f) * 0.025f;
}

ArtifactState resolveObsession(AethericObsession& obsession, StableId craftedItem,
                               StableId creator, std::uint64_t seed, TimeStamp time) {
    ArtifactState artifact{};
    artifact.artifact = makeDerivedId<ArtifactId>(seed, creator.value, 0x4145544845524943ULL, static_cast<std::uint64_t>(time.tick));
    artifact.item = craftedItem;
    artifact.creator = creator;
    artifact.name = "Aetheric Masterwork " + std::to_string(artifact.artifact.value & 0xFFFFFULL);
    artifact.motif = ContentId{"elysium:motif/aetheric_obsession"};
    artifact.value = 2500.0f + obsession.urgency * 5000.0f + obsession.progress * 2500.0f;
    artifact.indestructibleByOrdinaryMeans = obsession.urgency > 0.85f;
    obsession.resolved = true;
    obsession.progress = 1.0f;
    obsession.result = artifact.artifact;
    return artifact;
}

} // namespace elysium::fortress
