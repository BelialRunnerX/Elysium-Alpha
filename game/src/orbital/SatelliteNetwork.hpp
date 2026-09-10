#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track communications, navigation, weather, surveillance, defense, and science satellites around celestial bodies.
struct SatelliteNetworkCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct SatelliteNetworkRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class SatelliteNetworkService {
public:
    bool submit(const SatelliteNetworkCommand& command);
    const SatelliteNetworkRecord* lookup(std::uint64_t subjectId) const;
    std::vector<SatelliteNetworkRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, SatelliteNetworkRecord> records_;
};

}
