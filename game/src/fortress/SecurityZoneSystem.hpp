#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent access-controlled zones, alert levels, credentials, lockdowns, and intrusion incidents.
struct SecurityZoneSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct SecurityZoneSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class SecurityZoneSystemService {
public:
    bool submit(const SecurityZoneSystemCommand& command);
    const SecurityZoneSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<SecurityZoneSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, SecurityZoneSystemRecord> records_;
};

}
