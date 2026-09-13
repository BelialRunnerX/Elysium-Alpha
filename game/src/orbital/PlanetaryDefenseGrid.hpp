#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Coordinate surface batteries, orbital platforms, sensors, interceptors, and shield infrastructure.
struct PlanetaryDefenseGridCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct PlanetaryDefenseGridRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class PlanetaryDefenseGridService {
public:
    bool submit(const PlanetaryDefenseGridCommand& command);
    const PlanetaryDefenseGridRecord* lookup(std::uint64_t subjectId) const;
    std::vector<PlanetaryDefenseGridRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, PlanetaryDefenseGridRecord> records_;
};

}
