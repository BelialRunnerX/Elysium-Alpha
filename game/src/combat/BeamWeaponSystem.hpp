#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Resolve continuous and pulsed beam attacks through line queries, energy drain, heat, attenuation, and impact events.
struct BeamWeaponSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct BeamWeaponSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class BeamWeaponSystemService {
public:
    bool submit(const BeamWeaponSystemCommand& command);
    const BeamWeaponSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<BeamWeaponSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, BeamWeaponSystemRecord> records_;
};

}
