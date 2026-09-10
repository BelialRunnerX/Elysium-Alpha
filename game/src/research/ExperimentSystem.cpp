// Intended function: Represent experiments with hypotheses, apparatus, samples, hazard risk, reproducibility, and knowledge outputs.
#include "ExperimentSystem.hpp"

namespace elysium::research {

std::uint64_t ExperimentRunStore::keyOf(const ExperimentRun& value) noexcept { return static_cast<std::uint64_t>(value.runId); }

bool ExperimentRunStore::upsert(ExperimentRun value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const ExperimentRun& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool ExperimentRunStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const ExperimentRun& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const ExperimentRun* ExperimentRunStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const ExperimentRun& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<ExperimentRun> ExperimentRunStore::ordered() const { return records_; }

} // namespace elysium::research
