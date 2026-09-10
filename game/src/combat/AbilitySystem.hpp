#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Resolve data-driven activated abilities through cooldown, cost, targeting, effect, and authoritative command seams.
struct AbilitySystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct AbilitySystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class AbilitySystemService {
public:
    bool submit(const AbilitySystemCommand& command);
    const AbilitySystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<AbilitySystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, AbilitySystemRecord> records_;
};

}
