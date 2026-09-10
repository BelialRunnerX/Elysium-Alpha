#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate civilization archetypes, values, institutions, aesthetics, and technological biases from stable seeds.
struct CivilizationGeneratorRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct CivilizationGeneratorState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class CivilizationGeneratorSystem {
public:
    bool apply(const CivilizationGeneratorRequest& request);
    bool erase(std::uint64_t targetId);
    const CivilizationGeneratorState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, CivilizationGeneratorState> states_;
};

} // namespace elysium
