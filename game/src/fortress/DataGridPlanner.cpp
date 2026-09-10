#include "fortress/DataGridPlanner.hpp"

#include <algorithm>

namespace elysium {

// Intended function: Plan sensors, relays, terminals, automation links, and bandwidth priorities across local infrastructure.
bool DataGridPlannerService::submit(const DataGridPlannerCommand& command) {
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

const DataGridPlannerRecord* DataGridPlannerService::lookup(std::uint64_t subjectId) const {
    auto it = records_.find(subjectId);
    return it == records_.end() ? nullptr : &it->second;
}

std::vector<DataGridPlannerRecord> DataGridPlannerService::snapshot() const {
    std::vector<DataGridPlannerRecord> out; out.reserve(records_.size());
    for (const auto& [id, r] : records_) if (r.enabled) out.push_back(r);
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b){ return a.subjectId < b.subjectId; });
    return out;
}

bool DataGridPlannerService::remove(std::uint64_t subjectId) { return records_.erase(subjectId) != 0; }
void DataGridPlannerService::reset() { records_.clear(); revision_ = 1; }

}
