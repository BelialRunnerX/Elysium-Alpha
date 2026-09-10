#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Estimate sound obstruction and room transmission from bounded world/portal queries for presentation only.
struct AudioOcclusionCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct AudioOcclusionRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class AudioOcclusionService {
public:
    bool submit(const AudioOcclusionCommand& command);
    const AudioOcclusionRecord* lookup(std::uint64_t subjectId) const;
    std::vector<AudioOcclusionRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, AudioOcclusionRecord> records_;
};

}
