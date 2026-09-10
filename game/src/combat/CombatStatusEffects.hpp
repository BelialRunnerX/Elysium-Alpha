#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Apply bounded duration stacks for burns, bleeding, stun, slow, corrosion, radiation, toxins, and buffs.
struct CombatStatusEffectsCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct CombatStatusEffectsRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class CombatStatusEffectsService {
public:
    bool submit(const CombatStatusEffectsCommand& command);
    const CombatStatusEffectsRecord* lookup(std::uint64_t subjectId) const;
    std::vector<CombatStatusEffectsRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, CombatStatusEffectsRecord> records_;
};

}
