// Intended function: Represent experiments with hypotheses, apparatus, samples, hazard risk, reproducibility, and knowledge outputs.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::research {

struct ExperimentRun {
    std::uint64_t runId{};
    std::uint64_t experimentId{};
    double progress{};
    double requiredWork{};
    double confidence{};
    double risk{};
};

class ExperimentRunStore {
public:
    bool upsert(ExperimentRun value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const ExperimentRun* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<ExperimentRun> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const ExperimentRun& value) noexcept;
    std::vector<ExperimentRun> records_;
};

} // namespace elysium::research
