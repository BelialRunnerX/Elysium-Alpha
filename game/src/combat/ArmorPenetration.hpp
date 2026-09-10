#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Resolve layered armor penetration, ricochet, spall, heat, and material response into damage events.
struct ArmorPenetrationCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct ArmorPenetrationRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class ArmorPenetrationService {
public:
    bool submit(const ArmorPenetrationCommand& command);
    const ArmorPenetrationRecord* lookup(std::uint64_t subjectId) const;
    std::vector<ArmorPenetrationRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, ArmorPenetrationRecord> records_;
};

}
