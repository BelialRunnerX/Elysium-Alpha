#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Compose flora growth forms, climate tolerances, resources, hazards, reproduction, and visual tags.
struct PlantArchetypeGeneratorRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct PlantArchetypeGeneratorState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class PlantArchetypeGeneratorSystem {
public:
    bool apply(const PlantArchetypeGeneratorRequest& request);
    bool erase(std::uint64_t targetId);
    const PlantArchetypeGeneratorState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, PlantArchetypeGeneratorState> states_;
};

} // namespace elysium
