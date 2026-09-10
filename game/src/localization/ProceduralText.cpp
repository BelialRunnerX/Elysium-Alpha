#include "localization/ProceduralText.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Assemble localized procedural names, descriptions, Chronicle entries, item lore, and mission text from stable tokens.
bool ProceduralTextService::submit(const ProceduralTextCommand& command) {
    if (command.subjectId == 0) return false;
    auto& r = records_[command.subjectId];
    r.revision = revision_++;
    r.sourceId = command.sourceId;
    r.subjectId = command.subjectId;
    r.auxId = command.auxId;
    r.scalar = command.scalar;
    r.mode = command.mode;
    r.enabled = true;
    return true;
}

const ProceduralTextRecord* ProceduralTextService::lookup(std::uint64_t subjectId) const {
    auto it = records_.find(subjectId);
    return it == records_.end() ? nullptr : &it->second;
}

std::vector<ProceduralTextRecord> ProceduralTextService::snapshot() const {
    std::vector<ProceduralTextRecord> out; out.reserve(records_.size());
    for (const auto& [id, r] : records_) if (r.enabled) out.push_back(r);
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b){ return a.subjectId < b.subjectId; });
    return out;
}

bool ProceduralTextService::remove(std::uint64_t subjectId) { return records_.erase(subjectId) != 0; }
void ProceduralTextService::reset() { records_.clear(); revision_ = 1; }

}
