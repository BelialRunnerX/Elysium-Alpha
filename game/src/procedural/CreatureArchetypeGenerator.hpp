#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Compose fauna body plans, locomotion, senses, diet, defenses, behaviors, and loot ecology from deterministic seeds.
struct CreatureArchetypeGeneratorRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct CreatureArchetypeGeneratorState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class CreatureArchetypeGeneratorSystem {
public:
    bool apply(const CreatureArchetypeGeneratorRequest& request);
    bool erase(std::uint64_t targetId);
    const CreatureArchetypeGeneratorState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, CreatureArchetypeGeneratorState> states_;
};

} // namespace elysium
