#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Advance refinery, fabrication, shipyard, fuel, storage, and research operations using aggregate orbital logistics.
struct OrbitalIndustryCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct OrbitalIndustryRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class OrbitalIndustryService {
public:
    bool submit(const OrbitalIndustryCommand& command);
    const OrbitalIndustryRecord* lookup(std::uint64_t subjectId) const;
    std::vector<OrbitalIndustryRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, OrbitalIndustryRecord> records_;
};

}
