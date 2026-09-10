#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Resolve visibility, sound, heat, sensors, camouflage, lighting, movement, and environmental concealment.
struct StealthDetectionCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct StealthDetectionRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class StealthDetectionService {
public:
    bool submit(const StealthDetectionCommand& command);
    const StealthDetectionRecord* lookup(std::uint64_t subjectId) const;
    std::vector<StealthDetectionRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, StealthDetectionRecord> records_;
};

}
