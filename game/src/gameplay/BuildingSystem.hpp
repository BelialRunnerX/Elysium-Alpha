// Intended function: Represent blueprint placement, material staging, validation blockers, construction progress, and final world commits.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::gameplay {

struct BuildJob {
    std::uint64_t jobId{};
    std::uint64_t blueprintId{};
    std::uint64_t anchorAddressKey{};
    double progress{};
    double requiredWork{};
    double materialCost{};
    std::uint64_t state{};
};

class BuildJobStore {
public:
    bool upsert(BuildJob value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const BuildJob* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<BuildJob> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const BuildJob& value) noexcept;
    std::vector<BuildJob> records_;
};

} // namespace elysium::gameplay
