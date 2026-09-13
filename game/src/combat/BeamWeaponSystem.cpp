#include "combat/BeamWeaponSystem.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Resolve continuous and pulsed beam attacks through line queries, energy drain, heat, attenuation, and impact events.
bool BeamWeaponSystemService::submit(const BeamWeaponSystemCommand& command) {
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

const BeamWeaponSystemRecord* BeamWeaponSystemService::lookup(std::uint64_t subjectId) const {
    auto it = records_.find(subjectId);
    return it == records_.end() ? nullptr : &it->second;
}

std::vector<BeamWeaponSystemRecord> BeamWeaponSystemService::snapshot() const {
    std::vector<BeamWeaponSystemRecord> out; out.reserve(records_.size());
    for (const auto& [id, r] : records_) if (r.enabled) out.push_back(r);
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b){ return a.subjectId < b.subjectId; });
    return out;
}

bool BeamWeaponSystemService::remove(std::uint64_t subjectId) { return records_.erase(subjectId) != 0; }
void BeamWeaponSystemService::reset() { records_.clear(); revision_ = 1; }

}
