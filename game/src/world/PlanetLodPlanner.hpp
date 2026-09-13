// Intended function: select sparse planet representation tiers from distance, projected size, edit importance, and budget.
#pragma once

#include "core/Saturating.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::world {

struct LodRequest {
    std::uint64_t addressKey{};
    double distanceBand{};
    std::uint64_t lodTier{};
    std::uint64_t priority{};
    double estimatedCost{};
    std::uint64_t flags{};
};

struct LodInput {
    std::uint64_t addressKey{};
    double distanceMeters{};
    double planetRadiusMeters{};
    double projectedScreenFraction{};
    double editImportance{};
    double hazardImportance{};
    bool containsPlayerAuthoredChange{};
    bool activePhysics{};
};

struct LodDecision {
    LodRequest request{};
    std::uint64_t sampleStride{1};
    bool requireMacroVoxels{};
    bool requireMicroDetail{};
    bool preserveEditProxy{};
};

class PlanetLodSelector final {
public:
    [[nodiscard]] LodDecision decide(const LodInput& input) const noexcept;
};

class LodRequestStore {
public:
    bool upsert(LodRequest value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const LodRequest* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<LodRequest> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const LodRequest& value) noexcept;
    std::vector<LodRequest> records_;
};

} // namespace elysium::world
