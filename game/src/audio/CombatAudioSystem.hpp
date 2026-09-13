#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate weapon, impact, shield, explosion, suppression, injury, and tactical feedback audio events.
struct CombatAudioSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct CombatAudioSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class CombatAudioSystemService {
public:
    bool submit(const CombatAudioSystemCommand& command);
    const CombatAudioSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<CombatAudioSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, CombatAudioSystemRecord> records_;
};

}
