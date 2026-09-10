#include "fortress/FileKnowledgeSystems.hpp"

#include <algorithm>
#include <cmath>

namespace elysium::fortress {

void observeFileFact(FileKnowledge& file, FileFact fact) {
    auto it = std::find_if(file.facts.begin(), file.facts.end(), [&](const FileFact& existing) {
        return existing.kind == fact.kind && existing.subject == fact.subject && existing.system == fact.system && existing.site == fact.site;
    });
    if (it == file.facts.end()) {
        file.facts.push_back(std::move(fact));
        return;
    }
    if (fact.observed.tick >= it->observed.tick || fact.confidence > it->confidence) {
        const float oldConfidence = it->confidence;
        *it = std::move(fact);
        it->confidence = saturate(std::max(it->confidence, oldConfidence * 0.85f));
    }
}

float fileConfidence(const FileKnowledge& file, FileFactKind kind, StableId subject,
                     std::uint64_t system) {
    float best{};
    for (const auto& fact : file.facts) {
        if (fact.kind != kind || fact.subject != subject || fact.system != system || fact.stale) continue;
        best = std::max(best, fact.confidence);
    }
    return best;
}

void ageFileKnowledge(FileKnowledge& file, float days) {
    const float d = std::max(0.0f, days);
    for (auto& fact : file.facts) {
        fact.confidence *= std::exp(-d / 720.0f);
        if (fact.confidence < 0.12f) fact.stale = true;
    }
}

} // namespace elysium::fortress
