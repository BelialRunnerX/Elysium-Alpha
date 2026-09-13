// Intended function: Prioritize sparse chunk residency using player distance, active jobs, machines, hazards, edits, and save pressure.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::world {

struct ChunkResidencyRequest {
    std::uint64_t addressKey{};
    std::uint64_t priority{};
    std::uint64_t reasonFlags{};
    double memoryCost{};
    std::uint64_t lastTouchedTick{};
    std::uint64_t desiredState{};
};

class ChunkResidencyRequestStore {
public:
    bool upsert(ChunkResidencyRequest value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const ChunkResidencyRequest* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<ChunkResidencyRequest> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const ChunkResidencyRequest& value) noexcept;
    std::vector<ChunkResidencyRequest> records_;
};

} // namespace elysium::world
