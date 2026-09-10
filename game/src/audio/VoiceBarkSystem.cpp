#include "audio/VoiceBarkSystem.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Select deterministic contextual citizen, companion, squad, trader, and enemy bark intents without owning voice assets.
bool VoiceBarkSystemService::submit(const VoiceBarkSystemCommand& command) {
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

const VoiceBarkSystemRecord* VoiceBarkSystemService::lookup(std::uint64_t subjectId) const {
    auto it = records_.find(subjectId);
    return it == records_.end() ? nullptr : &it->second;
}

std::vector<VoiceBarkSystemRecord> VoiceBarkSystemService::snapshot() const {
    std::vector<VoiceBarkSystemRecord> out; out.reserve(records_.size());
    for (const auto& [id, r] : records_) if (r.enabled) out.push_back(r);
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b){ return a.subjectId < b.subjectId; });
    return out;
}

bool VoiceBarkSystemService::remove(std::uint64_t subjectId) { return records_.erase(subjectId) != 0; }
void VoiceBarkSystemService::reset() { records_.clear(); revision_ = 1; }

}
