#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track leisure venues, social activities, hobbies, and morale recovery opportunities in settlements.
struct RecreationSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct RecreationSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class RecreationSystemService {
public:
    bool submit(const RecreationSystemCommand& command);
    const RecreationSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<RecreationSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, RecreationSystemRecord> records_;
};

}
