#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Model personal, vehicle, structure, and ship shields with recharge, overload, damage typing, and power demand.
struct ShieldSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct ShieldSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class ShieldSystemService {
public:
    bool submit(const ShieldSystemCommand& command);
    const ShieldSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<ShieldSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, ShieldSystemRecord> records_;
};

}
