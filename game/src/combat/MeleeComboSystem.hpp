#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent timed melee attack chains, stamina costs, parries, interrupts, reach, and impact intents.
struct MeleeComboSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct MeleeComboSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class MeleeComboSystemService {
public:
    bool submit(const MeleeComboSystemCommand& command);
    const MeleeComboSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<MeleeComboSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, MeleeComboSystemRecord> records_;
};

}
