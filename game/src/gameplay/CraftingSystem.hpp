// Intended function: Drive data-oriented crafting jobs with recipe identity, reserved inputs, progress, output quality, and cancellation.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::gameplay {

struct CraftingJob {
    std::uint64_t jobId{};
    std::uint64_t recipeId{};
    std::uint64_t stationId{};
    double progress{};
    double requiredWork{};
    double qualityBias{};
    std::uint64_t state{};
};

class CraftingJobStore {
public:
    bool upsert(CraftingJob value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const CraftingJob* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<CraftingJob> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const CraftingJob& value) noexcept;
    std::vector<CraftingJob> records_;
};

} // namespace elysium::gameplay
