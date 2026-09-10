#include "fortress/Systems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

void advanceBiology(BiologicalState& state, float days, bool sleeping, bool eating, float atmosphereOxygen) {
    const float d = std::max(0.0f, days);
    state.hydration = saturate(state.hydration - d * (sleeping ? 0.16f : 0.24f));
    state.nutrition = saturate(state.nutrition - d * (sleeping ? 0.10f : 0.18f));
    if (eating) {
        state.hydration = saturate(state.hydration + d * 2.2f);
        state.nutrition = saturate(state.nutrition + d * 2.0f);
    }
    state.sleepDebt = std::clamp(state.sleepDebt + d * (sleeping ? -1.3f : 0.65f), 0.0f, 2.0f);
    state.oxygenation = saturate(state.oxygenation + (atmosphereOxygen - state.oxygenation) * saturate(d * 12.0f));
    state.pain = std::max(0.0f, state.pain - d * 0.12f * state.immunity);
}

void advanceNeeds(Needs& needs, float days, const Personality& personality, const Values& values) {
    const float d = std::max(0.0f, days);
    for (auto& need : needs.states) {
        float personalityScale = 1.0f;
        float valueScale = 1.0f;
        switch (need.kind) {
            case NeedKind::Social: personalityScale += personality.sociability * 0.6f; break;
            case NeedKind::Solitude: personalityScale += (1.0f - personality.sociability) * 0.5f; break;
            case NeedKind::Excitement: personalityScale += personality.riskTolerance * 0.4f; break;
            case NeedKind::Learning: personalityScale += personality.curiosity * 0.6f; valueScale += values.knowledge * 0.4f; break;
            case NeedKind::Family: valueScale += values.family * 0.7f; break;
            case NeedKind::Purpose: valueScale += (values.craft + values.martialHonor + values.knowledge) / 6.0f; break;
            case NeedKind::Safety: personalityScale += personality.anxiety * 0.6f; break;
            case NeedKind::Worship: valueScale += values.tradition * 0.35f; break;
            case NeedKind::Creativity: valueScale += values.craft * 0.4f; break;
            case NeedKind::Sleep:
            case NeedKind::Food: break;
        }
        need.satisfaction = saturate(need.satisfaction - need.decayPerDay * d * personalityScale * valueScale);
        need.urgency = saturate((1.0f - need.satisfaction) * personalityScale);
    }
}

StressBand classifyStress(float load) {
    if (load < 0.28f) return StressBand::Stable;
    if (load < 0.48f) return StressBand::Strained;
    if (load < 0.72f) return StressBand::Distressed;
    if (load < 0.88f) return StressBand::Crisis;
    return StressBand::Recovery;
}

void updateStress(StressState& stress, const Needs& needs, const Memories& memories,
                  const Personality& personality, float safety, float treatment, float days) {
    const float d = std::max(0.0f, days);
    float unmet{};
    std::vector<std::pair<float, std::string>> reasons;
    for (const auto& need : needs.states) {
        const float contribution = need.urgency * need.urgency;
        unmet += contribution;
        if (contribution > 0.1f) {
            reasons.emplace_back(contribution, "unmet need #" + std::to_string(static_cast<unsigned>(need.kind)));
        }
    }
    if (!needs.states.empty()) unmet /= static_cast<float>(needs.states.size());

    float memoryLoad{};
    for (const auto& memory : memories.entries) {
        if (memory.valence < 0.0f) {
            const float c = -memory.valence * memory.strength * (1.0f + memory.trauma);
            memoryLoad += c;
            if (c > 0.25f) reasons.emplace_back(c, memory.key);
        }
    }
    memoryLoad = std::min(1.0f, memoryLoad * 0.12f);

    const float sensitivity = 0.65f + personality.anxiety * 0.65f + personality.anger * 0.15f;
    const float incoming = saturate((unmet * 0.6f + memoryLoad * 0.4f + (1.0f - saturate(safety)) * 0.35f) * sensitivity);
    const float recovery = (0.03f + saturate(treatment) * 0.16f + saturate(safety) * 0.05f) * d * stress.recoveryReserve;
    stress.load = saturate(stress.load + incoming * d * 0.45f - recovery);

    if (stress.band == StressBand::Crisis && stress.load < 0.62f) {
        stress.band = StressBand::Recovery;
    } else if (stress.band == StressBand::Recovery && stress.load < 0.34f) {
        stress.band = StressBand::Stable;
    } else if (stress.band != StressBand::Recovery) {
        stress.band = classifyStress(stress.load);
    }

    std::sort(reasons.begin(), reasons.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
    stress.topReasons.clear();
    for (std::size_t i = 0; i < std::min<std::size_t>(3, reasons.size()); ++i) stress.topReasons.push_back(reasons[i].second);
}

void updateFocus(FocusState& focus, const Needs& needs, const StressState& stress,
                 const BiologicalState& biology) {
    float needPenalty{};
    for (const auto& need : needs.states) needPenalty += need.urgency;
    if (!needs.states.empty()) needPenalty /= static_cast<float>(needs.states.size());
    const float bodyPenalty = (1.0f - biology.nutrition) * 0.2f + biology.sleepDebt * 0.22f + biology.pain * 0.25f + (1.0f - biology.oxygenation) * 0.45f;
    focus.current = saturate(1.0f - needPenalty * 0.38f - stress.load * 0.52f - bodyPenalty);
    focus.workMultiplier = 0.35f + focus.current * 0.65f;
    focus.combatMultiplier = 0.45f + focus.current * 0.55f;
}

LifeStage classifyLifeStage(float years, float expectedYears) {
    if (expectedYears <= 0.0f) return LifeStage::Adult;
    const float ratio = years / expectedYears;
    if (ratio < 0.14f) return LifeStage::Dependent;
    if (ratio < 0.22f) return LifeStage::Adolescent;
    if (ratio < 0.72f) return LifeStage::Adult;
    if (ratio < 1.25f) return LifeStage::Elder;
    return LifeStage::Dead;
}

float learningGain(float difficulty, float currentRank, float aptitude, float focus, float mentorship) {
    const float challenge = std::clamp(0.35f + difficulty - currentRank * 0.08f, 0.08f, 2.0f);
    const float diminishing = 1.0f / (1.0f + currentRank * 0.12f);
    return std::max(0.0f, challenge * diminishing * std::max(0.1f, aptitude) * saturate(focus) * (1.0f + std::max(0.0f, mentorship)));
}

float relationshipInteractionDelta(const RelationshipEdge& edge, const Thought& interaction,
                                   const Personality& personality) {
    const float familiarityDamping = 1.0f / (1.0f + edge.familiarity * 0.2f);
    const float socialSensitivity = 0.6f + personality.sociability * 0.4f + personality.altruism * 0.2f;
    return interaction.valence * interaction.intensity * familiarityDamping * socialSensitivity;
}

void decayRelationship(RelationshipEdge& edge, float days) {
    const float d = std::max(0.0f, days);
    const float socialDecay = std::exp(-d / 720.0f);
    const float grievanceDecay = std::exp(-d / 1800.0f);
    edge.affinity *= socialDecay;
    edge.trust *= 0.9995f + 0.0005f * socialDecay;
    edge.fear *= socialDecay;
    edge.grievance *= grievanceDecay;
    edge.familiarity = std::max(0.0f, edge.familiarity - d / 3650.0f);
}

} // namespace elysium::fortress
